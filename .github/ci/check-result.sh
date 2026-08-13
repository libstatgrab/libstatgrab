#!/bin/sh
# Fail the host-side GitHub step with the status captured by run-target.sh.
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <platform-slug>" >&2
    exit 2
fi

slug=$1
status_file=".ci-output/$slug/exit-code"
if [ ! -r "$status_file" ]; then
    echo "No target status was copied back for $slug" >&2
    exit 1
fi

rc=$(cat "$status_file")
case "$rc" in
    ''|*[!0-9]*)
        echo "Invalid target status for $slug: $rc" >&2
        exit 1
        ;;
esac

if [ "$rc" -ne 0 ]; then
    echo "$slug failed inside its guest with exit code $rc." >&2
    for log in deps.log normal.log tests.log warnings.log config.normal.log config.tests.log; do
        file=".ci-output/$slug/$log"
        if [ -s "$file" ]; then
            echo "===== $slug: tail of $log =====" >&2
            tail -n 80 "$file" >&2 || true
        fi
    done
    echo "Full diagnostics are in the uploaded platform-$slug artifact." >&2
    # Shell exit statuses are 0..255. build-and-test normally returns a small
    # value, but avoid surprising modulo behaviour if the file is corrupted.
    if [ "$rc" -gt 255 ]; then
        exit 1
    fi
    exit "$rc"
fi
