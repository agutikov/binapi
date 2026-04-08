#!/usr/bin/env bash
# Run all REST demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Usage: scripts/testnet_rest.sh [output_dir]
#   output_dir defaults to testnet_output/rest

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/demo-cli/binapi2-fapi-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/rest}"
mkdir -p "$OUT"

SYMBOL=BTCUSDT

run() {
    local name="$1"
    shift
    echo "--- $name ---"
    "$CLI" \
        -S "$OUT/${name}.request" \
        -R "$OUT/${name}.response" \
        -L "$OUT/${name}.log" \
        -F trace \
        -v \
        "$@" 2>&1 | tail -1
    echo ""
}

echo "=== Public REST endpoints ==="

run ping              ping
run time              time
run exchange-info     exchange-info
run exchange-info-sym exchange-info "$SYMBOL"
run order-book        order-book "$SYMBOL" 5
run recent-trades     recent-trades "$SYMBOL"
run book-ticker       book-ticker "$SYMBOL"
run book-tickers      book-tickers
run price-ticker      price-ticker "$SYMBOL"
run price-tickers     price-tickers
run ticker-24hr       ticker-24hr "$SYMBOL"
run mark-price        mark-price "$SYMBOL"
run mark-prices       mark-prices
run klines            klines "$SYMBOL" 1h 5
run funding-rate      funding-rate "$SYMBOL" 5
run open-interest     open-interest "$SYMBOL"

echo "=== Authenticated REST endpoints ==="
if [ -z "${BINANCE_API_KEY:-}" ]; then
    echo "BINANCE_API_KEY not set, skipping auth endpoints."
    echo "Export BINANCE_API_KEY and BINANCE_SECRET_KEY to run these."
    exit 0
fi

run account-info      account-info
run balances          balances
run position-risk     position-risk
run position-risk-sym position-risk "$SYMBOL"
run income-history    income-history
run open-orders       open-orders
run test-order        test-order "$SYMBOL" BUY LIMIT -q 0.001 -p 10000 -t GTC

echo "=== Done ==="
echo "Output in: $OUT"
