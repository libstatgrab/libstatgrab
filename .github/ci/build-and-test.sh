#!/bin/sh
# Run the functional parts of the old GitLab matrix:
#   1. normal examples build + statgrab smoke/regression capture
#   2. clean --enable-tests build + make check (unless CI_RUN_TESTS=0)
#
# The advisory -Wall -Werror build is intentionally a separate GitHub job;
# see warning-build.sh. This keeps functional failures and compiler warnings
# distinct in the Actions UI, matching the old GitLab CI layout.
set -eu

if [ "$#" -ne 2 ]; then
    echo "usage: $0 <platform-slug> <libstatgrab-release.tar.gz>" >&2
    exit 2
fi

slug=$1
distfile=$2
root=$(pwd)
case "$distfile" in
    /*) ;;
    *) distfile="$root/$distfile" ;;
esac

if [ ! -f "$distfile" ]; then
    echo "Distribution tarball not found: $distfile" >&2
    exit 2
fi

out="$root/.ci-output/$slug"
work="$root/.ci-work/$slug"
# run-target.sh may already have written deps.log into the output
# directory.  Keep it while starting each build from a clean tree.
rm -rf "$work"
mkdir -p "$out" "$work"

# Capture the actual kernel and compiler used.  Do not compare this file in the
# regression step: patch-level kernel changes are expected on hosted images.
{
    uname -a
    if [ -r /etc/os-release ]; then
        echo "--- /etc/os-release ---"
        cat /etc/os-release
    fi
    if command -v freebsd-version >/dev/null 2>&1; then
        echo "--- freebsd-version ---"
        freebsd-version -ku 2>/dev/null || freebsd-version 2>/dev/null || true
    fi
    if command -v sw_vers >/dev/null 2>&1; then
        echo "--- sw_vers ---"
        sw_vers
    fi
} > "$out/platform.txt" 2>&1

compiler=${CC:-cc}
if ! command -v "$compiler" >/dev/null 2>&1; then
    if command -v gcc >/dev/null 2>&1; then
        compiler=gcc
    elif command -v clang >/dev/null 2>&1; then
        compiler=clang
    else
        echo "No C compiler found" >&2
        exit 1
    fi
fi
{
    echo "compiler=$compiler"
    "$compiler" --version 2>&1 || "$compiler" -V 2>&1 || true
} > "$out/compiler.txt"

make_cmd=${MAKE:-make}
if [ -z "${MAKE:-}" ] && [ "$(uname -s)" = SunOS ] && command -v gmake >/dev/null 2>&1; then
    make_cmd=gmake
fi
echo "make=$make_cmd" >> "$out/compiler.txt"
"$make_cmd" --version >> "$out/compiler.txt" 2>&1 || true

# DragonFly's third-party headers/libraries live under /usr/local.  The old
# Vagrant CI supplied these paths explicitly, and current pkg-installed curses
# and other compatibility libraries still follow that convention.
if [ "$(uname -s)" = DragonFly ]; then
    CPPFLAGS="${CPPFLAGS:+$CPPFLAGS }-I/usr/local/include"
    LDFLAGS="${LDFLAGS:+$LDFLAGS }-L/usr/local/lib"
    export CPPFLAGS LDFLAGS
fi

topdir=$(gzip -dc "$distfile" | tar tf - | sed -n '1{s,/.*,,;p;}')
if [ -z "$topdir" ]; then
    echo "Could not determine top-level directory in $distfile" >&2
    exit 1
fi

extract_tree()
{
    dest=$1
    rm -rf "$dest"
    mkdir -p "$dest"
    (cd "$dest" && gzip -dc "$distfile" | tar xf -)
    printf '%s/%s\n' "$dest" "$topdir"
}

copy_if_present()
{
    src=$1
    dst=$2
    if [ -f "$src" ]; then
        cp "$src" "$dst"
    fi
}

normal_src=$(extract_tree "$work/normal")
set +e
(
    set -e
    cd "$normal_src"
    ./configure --enable-examples
    "$make_cmd"

    : > "$out/smoke.txt"
    for stat in cpu. mem. load. user. swap. general. fs. disk. net. page.; do
        echo "===== $stat =====" >> "$out/smoke.txt"
        src/statgrab/statgrab "$stat" >> "$out/smoke.txt" 2>&1
    done

    if grep -F "Unknown stat" "$out/smoke.txt" >/dev/null 2>&1; then
        echo "statgrab reported an unknown statistic" >&2
        exit 1
    fi
    if grep -F "disabling saidar" config.log >/dev/null 2>&1; then
        echo "configure disabled saidar" >&2
        exit 1
    fi

    src/statgrab/statgrab > "$out/statgrab"
    cp config.h "$out/config.h"
) > "$out/normal.log" 2>&1
normal_rc=$?
set -e
copy_if_present "$normal_src/config.log" "$out/config.normal.log"
if [ "$normal_rc" -ne 0 ]; then
    cat "$out/normal.log" >&2
    exit "$normal_rc"
fi

if [ "${CI_RUN_TESTS:-1}" = 0 ]; then
    printf '%s\n' "skipped: CI_RUN_TESTS=0" > "$out/tests.skipped"
    printf '%s\n' "ok" > "$out/result.txt"
    echo "Completed build and smoke checks for $slug (make check skipped)"
    exit 0
fi

test_src=$(extract_tree "$work/tests")
set +e
(
    set -e
    cd "$test_src"
    ./configure --enable-tests
    "$make_cmd" check
) > "$out/tests.log" 2>&1
test_rc=$?
set -e
copy_if_present "$test_src/config.log" "$out/config.tests.log"
copy_if_present "$test_src/test-suite.log" "$out/test-suite.log"
copy_if_present "$test_src/tests/test-suite.log" "$out/test-suite.tests.log"
if [ "$test_rc" -ne 0 ]; then
    cat "$out/tests.log" >&2
    exit "$test_rc"
fi

printf '%s\n' "ok" > "$out/result.txt"
echo "Completed CI checks for $slug"
