#!/usr/bin/env bash
# Run all WebSocket API demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Usage: scripts/testnet/ws_api.sh [output_dir]
#   output_dir defaults to testnet_output/ws_api

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/ws_api}"
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

echo "=== WebSocket API — Session ==="

run ws-logon              ws-logon

echo "=== WebSocket API — Public ==="

run ws-book-ticker        ws-book-ticker "$SYMBOL"
run ws-book-ticker-all    ws-book-ticker
run ws-price-ticker       ws-price-ticker "$SYMBOL"
run ws-price-ticker-all   ws-price-ticker

echo "=== WebSocket API — Account ==="

run ws-account-status     ws-account-status
run ws-account-status-v2  ws-account-status-v2
run ws-account-balance    ws-account-balance
run ws-position           ws-position
run ws-position-sym       ws-position "$SYMBOL"

echo "=== WebSocket API — Orders (may fail without balance) ==="

run ws-order-place        ws-order-place "$SYMBOL" BUY LIMIT -q 0.002 -p 60000 -t GTC || true

echo "=== WebSocket API — User Data Stream ==="

run ws-user-stream-start  ws-user-stream-start
run ws-user-stream-ping   ws-user-stream-ping
run ws-user-stream-stop   ws-user-stream-stop

echo "=== Done ==="
echo "Output in: $OUT"
