#!/bin/sh
# Regression handling compatible with the old GitLab check-results jobs.
set -eu

usage()
{
    echo "usage:" >&2
    echo "  $0 normalize <downloaded-artifacts-dir> <normalized-dir>" >&2
    echo "  $0 compare   <normalized-current-dir> <reference-dir>" >&2
    echo "  $0 merge     <normalized-current-dir> <reference-dir> <publish-dir>" >&2
    exit 2
}

# statgrab collection keys include a hardware- or guest-specific instance name:
#   disk.<device>.<field>
#   fs.<device>.<field>
#   net.<interface>.<field>
#
# Hosted runner hardware can legitimately change those middle components between
# runs.  Collapse them to a wildcard so the regression gate answers the question
# we care about: did this platform stop exposing a statistic field?  The exact
# statgrab.keys file is retained alongside the schema for diagnostics.
canonicalize_keys()
{
    awk -F. '
        ($1 == "disk" || $1 == "fs" || $1 == "net") && NF >= 3 {
            print $1 ".*." $NF
            next
        }
        { print }
    ' "$1" | LC_ALL=C sort -u
}

schema_for()
{
    dir=$1
    output=$2
    if [ -f "$dir/statgrab.schema" ]; then
        cp "$dir/statgrab.schema" "$output"
    elif [ -f "$dir/statgrab.keys" ]; then
        # Backwards compatibility with baselines created before statgrab.schema
        # was introduced.
        canonicalize_keys "$dir/statgrab.keys" > "$output"
    else
        return 1
    fi
}

[ "$#" -ge 1 ] || usage
mode=$1
shift

case "$mode" in
    normalize)
        [ "$#" -eq 2 ] || usage
        src=$1
        dst=$2
        rm -rf "$dst"
        mkdir -p "$dst"
        count=0
        skipped=0
        invalid=0
        for artifact in "$src"/platform-*; do
            [ -d "$artifact" ] || continue
            name=$(basename "$artifact")

            if [ ! -f "$artifact/exit-code" ]; then
                echo "::error title=Incomplete CI artifact::$name has no exit-code"
                invalid=1
                continue
            fi
            rc=$(tr -d '[:space:]' < "$artifact/exit-code")
            case "$rc" in
                ''|*[!0-9]*)
                    echo "::error title=Invalid CI artifact::$name has invalid exit-code '$rc'"
                    invalid=1
                    continue
                    ;;
            esac
            if [ "$rc" -ne 0 ]; then
                echo "::notice title=Skipping failed CI platform::$name exited with status $rc; excluded from regression comparison"
                skipped=$((skipped + 1))
                continue
            fi

            if [ ! -f "$artifact/statgrab" ] || [ ! -f "$artifact/config.h" ]; then
                echo "::error title=Incomplete successful CI artifact::$name exited 0 but lacks statgrab and/or config.h"
                invalid=1
                continue
            fi
            mkdir -p "$dst/$name"
            # Strip changing values and retain only aggregate user fields.  Keep
            # the exact instance keys for diagnostics, then derive a stable
            # capability schema which ignores device/interface identity.
            sed -e 's/ =.*$//' -e '/^fs.*:/d' "$artifact/statgrab" \
                | perl -ne 'unless($_=~/^user\./&&$_!~/^user\.names\s+/&&$_!~/^user\.num\s+/){print}' \
                | LC_ALL=C sort > "$dst/$name/statgrab.keys"
            canonicalize_keys "$dst/$name/statgrab.keys" > "$dst/$name/statgrab.schema"
            cp "$artifact/config.h" "$dst/$name/config.h"
            if [ -f "$artifact/platform.txt" ]; then
                cp "$artifact/platform.txt" "$dst/$name/platform.txt"
            fi
            count=$((count + 1))
        done
        if [ "$invalid" -ne 0 ]; then
            exit 1
        fi
        if [ "$count" -eq 0 ]; then
            echo "::error::No successful platform artifacts were available for regression comparison"
            exit 1
        fi
        echo "Normalized $count successful platform artifacts; skipped $skipped failed artifacts"
        ;;

    compare)
        [ "$#" -eq 2 ] || usage
        current=$1
        reference=$2
        if [ ! -d "$reference" ] || ! ls "$reference"/platform-* >/dev/null 2>&1; then
            echo "::notice::No CI regression baseline exists yet; the first successful default-branch run will create it."
            exit 0
        fi

        tmp="$current/.compare-schema.$$"
        rm -rf "$tmp"
        mkdir -p "$tmp"
        trap 'rm -rf "$tmp"' 0 1 2 3 15

        changed=0
        for platform in "$current"/platform-*; do
            [ -d "$platform" ] || continue
            name=$(basename "$platform")
            ref="$reference/$name"
            if [ ! -d "$ref" ]; then
                echo "::notice title=New CI platform::$name has no previous baseline"
                continue
            fi

            current_schema="$tmp/$name.current.schema"
            reference_schema="$tmp/$name.reference.schema"
            if ! schema_for "$platform" "$current_schema"; then
                echo "::error title=Incomplete CI regression input::$name has no current statgrab schema/keys"
                changed=1
                continue
            fi
            if ! schema_for "$ref" "$reference_schema"; then
                echo "::error title=Incomplete CI baseline::$name has no baseline statgrab schema/keys"
                changed=1
                continue
            fi

            schema_changed=0
            if ! cmp -s "$reference_schema" "$current_schema"; then
                echo "::error title=CI regression::$name changed statgrab capability schema"
                diff -u "$reference_schema" "$current_schema" || true
                changed=1
                schema_changed=1
            fi

            # Exact instance keys are useful context but are deliberately not a
            # gating signal: GitHub-hosted runner disks, filesystems and PCI-based
            # interface names can vary from one run to another.
            if [ "$schema_changed" -eq 0 ] && \
               [ -f "$ref/statgrab.keys" ] && [ -f "$platform/statgrab.keys" ] && \
               ! cmp -s "$ref/statgrab.keys" "$platform/statgrab.keys"; then
                removed=$(comm -23 "$ref/statgrab.keys" "$platform/statgrab.keys" | wc -l | tr -d '[:space:]')
                added=$(comm -13 "$ref/statgrab.keys" "$platform/statgrab.keys" | wc -l | tr -d '[:space:]')
                echo "::notice title=CI hardware inventory changed::$name changed instance keys ($removed removed, $added added), but its statistic schema is unchanged"
            fi

            if [ ! -f "$ref/config.h" ] || [ ! -f "$platform/config.h" ]; then
                echo "::error title=Incomplete CI regression input::$name lacks config.h in current result or baseline"
                changed=1
            elif ! cmp -s "$ref/config.h" "$platform/config.h"; then
                echo "::error title=CI regression::$name changed config.h"
                diff -u "$ref/config.h" "$platform/config.h" || true
                changed=1
            fi
        done
        exit "$changed"
        ;;

    merge)
        [ "$#" -eq 3 ] || usage
        current=$1
        reference=$2
        publish=$3
        rm -rf "$publish"
        mkdir -p "$publish"
        if [ -d "$reference" ]; then
            cp -R "$reference"/. "$publish"/
        fi
        for platform in "$current"/platform-*; do
            [ -d "$platform" ] || continue
            name=$(basename "$platform")
            rm -rf "$publish/$name"
            cp -R "$platform" "$publish/$name"
        done
        ;;

    *)
        usage
        ;;
esac
