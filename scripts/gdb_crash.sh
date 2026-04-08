#!/usr/bin/env bash
# Run a command under gdb, print all thread backtraces on crash.
#
# Usage: scripts/gdb_crash.sh <command> [args...]
# Example: scripts/gdb_crash.sh binapi2-fapi-demo-cli stream-book-ticker BTCUSDT

set -euo pipefail

if [ $# -eq 0 ]; then
    echo "Usage: $0 <command> [args...]"
    exit 1
fi

gdb -batch \
    -ex run \
    -ex "thread apply all bt full" \
    -ex quit \
    --args "$@"
