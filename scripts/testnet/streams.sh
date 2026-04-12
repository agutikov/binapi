#!/usr/bin/env bash
# Run all Market Stream demo-cli commands against Binance testnet.
# Each stream runs for a few seconds to capture sample events.
# Saves stream frames (JSONL) and log for each command.
#
# Usage: scripts/testnet/streams.sh [output_dir]
#   output_dir defaults to testnet_output/streams

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/streams}"
rm -rf "$OUT"
mkdir -p "$OUT"

SYMBOL=BTCUSDT
PAIR=BTCUSDT
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

echo "=== Market Streams — Single Symbol ==="

run_stream stream-aggregate-trade  stream-aggregate-trade "$SYMBOL"
run_stream stream-book-ticker      stream-book-ticker "$SYMBOL"
run_stream stream-mark-price       stream-mark-price "$SYMBOL"
run_stream stream-kline            stream-kline "$SYMBOL" 1m
run_stream stream-ticker           stream-ticker "$SYMBOL"
run_stream stream-mini-ticker      stream-mini-ticker "$SYMBOL"
run_stream stream-depth            stream-depth "$SYMBOL" 5
run_stream stream-diff-depth       stream-diff-depth "$SYMBOL"
run_stream stream-liquidation      stream-liquidation "$SYMBOL"
run_stream stream-composite-index  stream-composite-index "$SYMBOL"
run_stream stream-asset-index      stream-asset-index "$SYMBOL"
run_stream stream-continuous-kline stream-continuous-kline "$PAIR" 1m
run_stream stream-rpi-diff-depth   stream-rpi-diff-depth "$SYMBOL"

echo "=== Market Streams — All Symbols ==="

run_stream stream-all-book-tickers stream-all-book-tickers
run_stream stream-all-tickers      stream-all-tickers
run_stream stream-all-mini-tickers stream-all-mini-tickers
run_stream stream-all-liquidations stream-all-liquidations
run_stream stream-all-mark-prices  stream-all-mark-prices
run_stream stream-all-asset-index  stream-all-asset-index

echo "=== Market Streams — Events ==="

run_stream stream-contract-info    stream-contract-info
run_stream stream-trading-session  stream-trading-session

echo "=== Done ==="
echo "Output in: $OUT"
