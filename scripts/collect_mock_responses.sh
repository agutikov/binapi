#!/usr/bin/env bash
# Collect real Binance testnet responses to update postman mock fixtures.
#
# Fetches each response type from the testnet and saves it to
# compose/postman-mock/responses/. Only overwrites if the request succeeds.
#
# Usage: scripts/collect_mock_responses.sh
#
# For auth endpoints: export BINANCE_API_KEY and BINANCE_SECRET_KEY first.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/demo-cli/binapi2-fapi-demo-cli"
MOCK_DIR="$ROOT_DIR/compose/postman-mock/responses"
TMP_DIR=$(mktemp -d)

trap 'rm -rf "$TMP_DIR"' EXIT

SYMBOL=BTCUSDT
OK=0
FAIL=0
SKIP=0

collect() {
    local fixture="$1"
    local name="$2"
    shift 2
    echo -n "  $name -> $fixture ... "
    if "$CLI" -R "$TMP_DIR/$fixture" -L "$TMP_DIR/${name}.log" -F trace "$@" >/dev/null 2>&1; then
        if [ -s "$TMP_DIR/$fixture" ]; then
            cp "$TMP_DIR/$fixture" "$MOCK_DIR/$fixture"
            echo "OK"
            OK=$((OK + 1))
        else
            echo "EMPTY (skipped)"
            SKIP=$((SKIP + 1))
        fi
    else
        echo "FAIL (kept old)"
        FAIL=$((FAIL + 1))
    fi
}

echo "Collecting responses from Binance testnet..."
echo "Mock dir: $MOCK_DIR"
echo ""

echo "=== Public endpoints ==="
collect ping.json             ping              ping
collect server_time.json      time              time
collect exchange_info.json    exchange-info     exchange-info
collect depth.json            order-book        order-book "$SYMBOL" 5
collect trades.json           recent-trades     recent-trades "$SYMBOL"
collect ticker_book.json      book-ticker       book-ticker "$SYMBOL"
collect ticker_price.json     price-ticker      price-ticker "$SYMBOL"
collect premium_index.json    mark-price        mark-price "$SYMBOL"
collect funding_rate.json     funding-rate      funding-rate "$SYMBOL" 5
collect klines.json           klines            klines "$SYMBOL" 1h 5

echo ""
echo "=== Authenticated endpoints ==="
if [ -z "${BINANCE_API_KEY:-}" ]; then
    echo "  BINANCE_API_KEY not set — skipping auth endpoints."
    echo "  Export BINANCE_API_KEY and BINANCE_SECRET_KEY to collect these."
    SKIP=$((SKIP + 7))
else
    collect account.json       account-info      account-info
    collect balance.json       balances          balances
    collect position_risk.json position-risk     position-risk
    collect open_orders.json   open-orders       open-orders
    collect listen_key.json    listen-key-start  listen-key-start

    # test-order returns the order fixture shape
    collect order.json         test-order        test-order "$SYMBOL" BUY LIMIT -q 0.001 -p 10000 -t GTC
fi

echo ""
echo "=== Summary ==="
echo "  OK:      $OK"
echo "  Failed:  $FAIL (old fixture kept)"
echo "  Skipped: $SKIP"
echo ""
echo "Review changes: git diff compose/postman-mock/responses/"
