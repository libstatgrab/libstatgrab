#!/bin/sh
# Start a Lima guest with one clean retry. Hosted-runner image/network hiccups
# should not turn an otherwise healthy distro into a one-off CI failure.
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <lima-template>" >&2
    exit 2
fi

template=$1
attempt=1
max_attempts=2

while :; do
    echo "Starting Lima template:$template (attempt $attempt/$max_attempts)"
    if limactl start --plain --name=default --cpus=2 --memory=4 "template:$template"; then
        exit 0
    else
        rc=$?
    fi

    if [ "$attempt" -ge "$max_attempts" ]; then
        echo "Lima failed to start template:$template after $attempt attempts" >&2
        exit "$rc"
    fi

    echo "Lima start failed; deleting the partial instance before one retry" >&2
    limactl stop -f default >/dev/null 2>&1 || true
    limactl delete -f default >/dev/null 2>&1 || true
    sleep 5
    attempt=$((attempt + 1))
done
