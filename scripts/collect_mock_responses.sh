#!/usr/bin/env bash
# Copy real testnet responses into postman mock fixtures.
#
# Reads from testnet_output/rest/ (produced by testnet_rest.sh)
# and copies response.json files into compose/postman-mock/responses/.
#
# Usage:
#   scripts/testnet_rest.sh          # run first to collect responses
#   scripts/collect_mock_responses.sh # then copy into mock fixtures
#
# Only overwrites a fixture if the source response exists and is non-empty.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
INPUT="${1:-$ROOT_DIR/testnet_output/rest}"
MOCK_DIR="$ROOT_DIR/compose/postman-mock/responses"

OK=0
FAIL=0

copy() {
    local fixture="$1"
    local command="$2"
    local src="$INPUT/$command/response.json"
    echo -n "  $command -> $fixture ... "
    if [ -f "$src" ] && [ -s "$src" ]; then
        cp "$src" "$MOCK_DIR/$fixture"
        echo "OK"
        OK=$((OK + 1))
    else
        echo "MISSING (kept old)"
        FAIL=$((FAIL + 1))
    fi
}

echo "Copying testnet responses into mock fixtures..."
echo "  from: $INPUT"
echo "  to:   $MOCK_DIR"
echo ""

echo "=== Public endpoints ==="
copy ping.json           ping
copy server_time.json    time
copy exchange_info.json  exchange-info
copy depth.json          order-book
copy trades.json         recent-trades
copy ticker_book.json    book-ticker
copy ticker_price.json   price-ticker
copy premium_index.json  mark-price
copy funding_rate.json   funding-rate
copy klines.json         klines

echo ""
echo "=== Authenticated endpoints ==="
copy account.json        account-info
copy balance.json        balances
copy position_risk.json  position-risk
copy open_orders.json    open-orders
copy order.json          test-order
copy listen_key.json     listen-key-start

echo ""
echo "=== Summary ==="
echo "  Copied:  $OK"
echo "  Missing: $FAIL (old fixture kept)"
echo ""
echo "Review changes: git diff compose/postman-mock/responses/"
