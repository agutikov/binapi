#!/usr/bin/env bash
# Run all Account REST demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Usage: scripts/testnet/account.sh [output_dir]
#   output_dir defaults to testnet_output/account

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/account}"
rm -rf "$OUT"
mkdir -p "$OUT"

SYMBOL=BTCUSDT

run() {
    local name="$1"
    shift
    local dir="$OUT/$name"
    mkdir -p "$dir"
    echo -n "  $name ... "
    if "$CLI" \
        -v \
        -S "$dir/request" \
        -R "$dir/response.json" \
        -L "$dir/log.txt" \
        -F trace \
        -K "${BINAPI2_SECRET:-libsecret:demo}" \
        "$@" > "$dir/stdout.txt" 2>&1; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

echo "=== Account — Information ==="

run account-info      account-info
run balances          balances
run account-config    account-config

echo "=== Account — Position & Config ==="

run position-risk     position-risk
run position-risk-sym position-risk "$SYMBOL"
run symbol-config     symbol-config
run symbol-config-sym symbol-config "$SYMBOL"
run multi-assets-mode multi-assets-mode
run position-mode     position-mode

echo "=== Account — Rate Limits & Brackets ==="

run rate-limit-order  rate-limit-order
run leverage-bracket  leverage-bracket
run leverage-bracket-sym leverage-bracket "$SYMBOL"
run commission-rate   commission-rate "$SYMBOL"

echo "=== Account — BNB Burn & Rules ==="

run bnb-burn          bnb-burn
run quantitative-rules quantitative-rules
run quantitative-rules-sym quantitative-rules "$SYMBOL"

echo "=== Account — Income History ==="

run income-history    income-history
run income-history-sym income-history "$SYMBOL" 5

echo "=== Account — Portfolio Margin ==="

run pm-account-info   pm-account-info USDT || true

echo "=== Done ==="
echo "Output in: $OUT"
