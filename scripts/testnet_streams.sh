#!/usr/bin/env bash
# Run WebSocket stream demo-cli commands against Binance testnet.
# Each stream runs for a few seconds to capture sample events.
# Saves log for each stream command.
#
# Usage: scripts/testnet_streams.sh [output_dir]
#   output_dir defaults to testnet_output/streams

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/streams}"
mkdir -p "$OUT"

SYMBOL=BTCUSDT
TIMEOUT=5  # seconds per stream

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
            -v \
            "$@" \
        > "$dir/stdout.txt" 2>/dev/null || true) 2>/dev/null
    local frames=0
    if [ -f "$dir/stream.jsonl" ]; then
        frames=$(wc -l < "$dir/stream.jsonl")
    fi
    echo "$frames frames"
}

echo "=== WebSocket streams (public) ==="

run_stream stream-book-ticker      stream-book-ticker "$SYMBOL"
run_stream stream-mark-price       stream-mark-price "$SYMBOL"
run_stream stream-kline            stream-kline "$SYMBOL" 1m
run_stream stream-ticker           stream-ticker "$SYMBOL"
run_stream stream-depth            stream-depth "$SYMBOL" 5
run_stream stream-all-book-tickers stream-all-book-tickers
run_stream stream-all-tickers      stream-all-tickers
run_stream stream-all-mini-tickers stream-all-mini-tickers

echo "=== WebSocket streams (auth) ==="
if [ -z "${BINANCE_API_KEY:-}" ]; then
    echo "BINANCE_API_KEY not set, skipping user-stream."
else
    run_stream listen-key-start    listen-key-start
    run_stream user-stream         user-stream
fi

echo "=== Done ==="
echo "Output in: $OUT"
