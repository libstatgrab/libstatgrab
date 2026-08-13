#!/bin/sh
# Report the recorded -Wall -Werror result as an advisory GitHub warning.
# Compiler-warning failures deliberately do not fail the job: GitHub has no
# GitLab-style "allowed failure" job state, so a warning annotation is the
# closest equivalent without making the workflow look broadly broken.
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <platform-slug>" >&2
    exit 2
fi

slug=$1
out=".ci-warning-output/$slug"
status="$out/exit-code"

set_output() {
    if [ -n "${GITHUB_OUTPUT:-}" ]; then
        printf 'failed=%s\n' "$1" >> "$GITHUB_OUTPUT"
    fi
}

summary() {
    if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
        printf '%s\n' "$1" >> "$GITHUB_STEP_SUMMARY"
    fi
}

if [ ! -f "$status" ]; then
    echo "::warning title=Compiler warning build unavailable::$slug did not produce a warning-build result (VM/setup may have failed)"
    summary "⚠️ **$slug:** warning build result unavailable (VM/setup may have failed)."
    set_output true
    exit 0
fi

rc=$(cat "$status")
case "$rc" in
    ''|*[!0-9]*)
        echo "::warning title=Compiler warning build invalid::$slug produced invalid exit status '$rc'"
        summary "⚠️ **$slug:** warning build produced an invalid result."
        set_output true
        exit 0
        ;;
esac

if [ "$rc" -eq 0 ]; then
    echo "-Wall -Werror build passed on $slug"
    summary "✅ **$slug:** -Wall -Werror build passed."
    set_output false
    exit 0
fi

echo "::warning title=-Wall -Werror build failed::$slug failed the advisory compiler-warning build (exit $rc); functional CI is unaffected"
summary "⚠️ **$slug:** -Wall -Werror build failed (exit $rc); functional CI is unaffected."
for log in deps.log warnings.log config.warnings.log; do
    if [ -f "$out/$log" ]; then
        echo "===== tail: $log ====="
        tail -n 80 "$out/$log" || true
    fi
done
set_output true
exit 0
