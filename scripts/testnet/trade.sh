#!/usr/bin/env bash
# Run Trade REST demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Note: Most write operations (new-order, change-leverage, etc.) will likely
# fail on testnet without proper balance/state — FAIL is expected for those.
# test-order validates without placing.
#
# Usage: scripts/testnet/trade.sh [output_dir]
#   output_dir defaults to testnet_output/trade

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/trade}"
mkdir -p "$OUT"

SYMBOL=BTCUSDT

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

echo "=== Trade — Order Queries (read-only) ==="

run open-orders       open-orders
run open-orders-sym   open-orders "$SYMBOL"
run all-orders        all-orders "$SYMBOL" 5
run force-orders      force-orders
run account-trades    account-trades "$SYMBOL" 5

echo "=== Trade — Test Order ==="

run test-order        test-order "$SYMBOL" BUY LIMIT -q 0.001 -p 10000 -t GTC

echo "=== Trade — Position Queries (read-only) ==="

run position-info-v3  position-info-v3
run position-info-v3-sym position-info-v3 "$SYMBOL"
run adl-quantile      adl-quantile
run position-margin-history position-margin-history "$SYMBOL" 5
run order-modify-history order-modify-history "$SYMBOL"

echo "=== Trade — Algo Order Queries (read-only) ==="

run open-algo-orders  open-algo-orders

echo "=== Trade — Configuration (may fail without proper state) ==="

run change-leverage   change-leverage "$SYMBOL" 10 || true
run tradfi-perps      tradfi-perps || true

echo "=== Done ==="
echo "Output in: $OUT"
