#!/usr/bin/env bash
# Run Order Book demo-cli commands against Binance testnet.
# Tests live order book (REST snapshot + stream deltas) and pipelined variant.
#
# Usage: scripts/testnet/order_book.sh [output_dir]
#   output_dir defaults to testnet_output/order_book

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/order_book}"
rm -rf "$OUT"
mkdir -p "$OUT"

SYMBOL=BTCUSDT
TIMEOUT=10  # seconds per order book session

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

echo "=== Order Book — Live ==="

run_stream order-book-live          order-book-live "$SYMBOL" 10
run_stream pipeline-order-book-live pipeline-order-book-live "$SYMBOL" 10

echo "=== Done ==="
echo "Output in: $OUT"
