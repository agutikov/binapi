#!/usr/bin/env bash
# Run all Market Data REST demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Usage: scripts/testnet/market_data.sh [output_dir]
#   output_dir defaults to testnet_output/market_data

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/market_data}"
mkdir -p "$OUT"

SYMBOL=BTCUSDT
PAIR=BTCUSDT

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

echo "=== Market Data — Connectivity ==="

run ping              ping
run time              time

echo "=== Market Data — Exchange Info ==="

run exchange-info     exchange-info
run exchange-info-sym exchange-info "$SYMBOL"

echo "=== Market Data — Order Book & Trades ==="

run order-book        order-book "$SYMBOL" 5
run recent-trades     recent-trades "$SYMBOL" 5
run aggregate-trades  aggregate-trades "$SYMBOL" 5
run historical-trades historical-trades "$SYMBOL" 5

echo "=== Market Data — Tickers ==="

run book-ticker       book-ticker "$SYMBOL"
run book-tickers      book-tickers
run price-ticker      price-ticker "$SYMBOL"
run price-tickers     price-tickers
run price-ticker-v2   price-ticker-v2 "$SYMBOL"
run price-tickers-v2  price-tickers-v2
run ticker-24hr       ticker-24hr "$SYMBOL"
run ticker-24hrs      ticker-24hrs
run mark-price        mark-price "$SYMBOL"
run mark-prices       mark-prices

echo "=== Market Data — Klines ==="

run klines              klines "$SYMBOL" 1h 5
run continuous-kline    continuous-kline "$PAIR" 1h 5
run index-price-kline   index-price-kline "$PAIR" 1h 5
run mark-price-klines   mark-price-klines "$SYMBOL" 1h 5
run premium-index-klines premium-index-klines "$SYMBOL" 1h 5

echo "=== Market Data — Funding & Open Interest ==="

run funding-rate        funding-rate "$SYMBOL" 5
run funding-rate-info   funding-rate-info
run open-interest       open-interest "$SYMBOL"
run open-interest-stats open-interest-stats "$SYMBOL" 1d 5

echo "=== Market Data — Analytics ==="

run top-ls-account-ratio top-ls-account-ratio "$SYMBOL" 1d 5
run top-ls-trader-ratio  top-ls-trader-ratio "$SYMBOL" 1d 5
run long-short-ratio     long-short-ratio "$SYMBOL" 1d 5
run taker-volume         taker-volume "$SYMBOL" 1d 5
run basis                basis "$PAIR" 5m 5

echo "=== Market Data — Index & Misc ==="

run delivery-price       delivery-price "$PAIR"
run composite-index-info composite-index-info
run index-constituents   index-constituents "$SYMBOL"
run asset-index          asset-index
run insurance-fund       insurance-fund
run adl-risk             adl-risk
run rpi-depth            rpi-depth "$SYMBOL" 5
run trading-schedule     trading-schedule

echo "=== Done ==="
echo "Output in: $OUT"
