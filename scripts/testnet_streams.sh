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
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/demo-cli/binapi2-fapi-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/streams}"
mkdir -p "$OUT"

SYMBOL=BTCUSDT
TIMEOUT=5  # seconds per stream

run_stream() {
    local name="$1"
    shift
    echo "--- $name (${TIMEOUT}s) ---"
    timeout "$TIMEOUT" \
        "$CLI" \
            -L "$OUT/${name}.log" \
            -F trace \
            -v \
            "$@" \
        > "$OUT/${name}.stdout" 2>&1 || true
    local lines
    lines=$(wc -l < "$OUT/${name}.stdout")
    echo "  captured $lines lines"
    echo ""
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
