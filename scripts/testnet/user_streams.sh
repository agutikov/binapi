#!/usr/bin/env bash
# Run User Data Stream demo-cli commands against Binance testnet.
# Tests listen key lifecycle and user stream subscription.
#
# Usage: scripts/testnet/user_streams.sh [output_dir]
#   output_dir defaults to testnet_output/user_streams

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/user_streams}"
mkdir -p "$OUT"

TIMEOUT=5  # seconds for stream

run() {
    local name="$1"
    shift
    local dir="$OUT/$name"
    mkdir -p "$dir"
    echo -n "  $name ... "
    if "$CLI" \
        -S "$dir/request" \
        -R "$dir/response.json" \
        -L "$dir/log.txt" \
        -F trace \
        -K "${BINAPI2_SECRET:-libsecret:demo}" \
        "$@" >/dev/null 2>&1; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

run_stream() {
    local name="$1"
    shift
    local dir="$OUT/$name"
    mkdir -p "$dir"
    echo -n "  $name (${TIMEOUT}s) ... "
    (timeout "$TIMEOUT" \
        "$CLI" \
            -r "$dir/stream.jsonl" \
            -L "$dir/log.txt" \
            -F trace \
            -K "${BINAPI2_SECRET:-libsecret:demo}" \
            -v \
            "$@" \
        > "$dir/stdout.txt" 2>/dev/null || true) 2>/dev/null
    local frames=0
    if [ -f "$dir/stream.jsonl" ]; then
        frames=$(wc -l < "$dir/stream.jsonl")
    fi
    echo "$frames frames"
}

echo "=== User Data Streams — Listen Key Lifecycle ==="

run listen-key-start     listen-key-start
run listen-key-keepalive listen-key-keepalive
run listen-key-close     listen-key-close

echo "=== User Data Streams — Stream ==="

run_stream user-stream   user-stream

echo "=== Done ==="
echo "Output in: $OUT"
