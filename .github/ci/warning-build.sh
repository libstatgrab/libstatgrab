#!/bin/sh
# Run the advisory compiler-warning build for one platform. This is separate
# from build-and-test.sh so a -Wall -Werror failure is visible as its own
# non-blocking GitHub Actions job.
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

out="$root/.ci-warning-output/$slug"
work="$root/.ci-warning-work/$slug"
rm -rf "$work"
mkdir -p "$out" "$work"

{
    uname -a
    if [ -r /etc/os-release ]; then
        echo "--- /etc/os-release ---"
        cat /etc/os-release
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

mkdir -p "$work/src"
(cd "$work/src" && gzip -dc "$distfile" | tar xf -)
src="$work/src/$topdir"

set +e
(
    set -e
    cd "$src"
    if [ -n "${CFLAGS:-}" ]; then
        CFLAGS="$CFLAGS -Wall -Werror"
    else
        CFLAGS="-Wall -Werror"
    fi
    export CFLAGS
    ./configure --enable-examples
    "$make_cmd"
) > "$out/warnings.log" 2>&1
rc=$?
set -e

if [ -f "$src/config.log" ]; then
    cp "$src/config.log" "$out/config.warnings.log"
fi

if [ "$rc" -ne 0 ]; then
    cat "$out/warnings.log" >&2
fi
exit "$rc"
