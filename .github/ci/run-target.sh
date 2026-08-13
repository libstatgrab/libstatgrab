#!/bin/sh
# Run a platform target without allowing the VM wrapper to abort before it has
# synchronized .ci-output back to the GitHub runner. The real status is
# recorded and checked by check-result.sh on the host afterwards.
#
# Set CI_INSTALL_DEPS=1 for VM guests where dependency installation should be
# captured in the platform artifact too.
set -u

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <platform-slug>" >&2
    exit 2
fi

slug=$1
out=".ci-output/$slug"
mkdir -p "$out"

if [ "${CI_INSTALL_DEPS:-0}" = 1 ]; then
    dep_rc=0
    sh .github/ci/install-deps.sh > "$out/deps.log" 2>&1 || dep_rc=$?
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
sh .github/ci/build-and-test.sh "$slug" "$1" || rc=$?
printf '%s\n' "$rc" > "$out/exit-code"
exit 0
