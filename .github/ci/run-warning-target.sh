#!/bin/sh
# Run a warning-only target while preserving its logs/status for VM wrappers.
# The wrapper itself exits successfully so VM actions can synchronize the
# output; check-warning-result.sh turns the recorded failure into an advisory
# GitHub job failure afterwards.
set -u

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <platform-slug>" >&2
    exit 2
fi

slug=$1
out=".ci-warning-output/$slug"
mkdir -p "$out"

if [ "${CI_INSTALL_DEPS:-0}" = 1 ]; then
    dep_rc=0
    CI_RUN_TESTS=0 sh .github/ci/install-deps.sh > "$out/deps.log" 2>&1 || dep_rc=$?
    if [ "$dep_rc" -ne 0 ]; then
        cat "$out/deps.log" >&2
        printf '%s\n' "$dep_rc" > "$out/exit-code"
        exit 0
    fi
fi

set -- dist/libstatgrab-*.tar.gz
if [ "$#" -ne 1 ] || [ ! -f "$1" ]; then
    echo "Expected exactly one distribution tarball under dist/" >&2
    printf '%s\n' 2 > "$out/exit-code"
    exit 0
fi

rc=0
sh .github/ci/warning-build.sh "$slug" "$1" || rc=$?
printf '%s\n' "$rc" > "$out/exit-code"
exit 0
