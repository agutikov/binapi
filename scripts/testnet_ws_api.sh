#!/usr/bin/env bash
# Run WebSocket API demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Usage: scripts/testnet_ws_api.sh [output_dir]
#   output_dir defaults to testnet_output/ws_api
#
# Requires: BINANCE_API_KEY and BINANCE_SECRET_KEY

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/demo-cli/binapi2-fapi-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/ws_api}"
mkdir -p "$OUT"

if [ -z "${BINANCE_API_KEY:-}" ]; then
    echo "BINANCE_API_KEY not set. Export BINANCE_API_KEY and BINANCE_SECRET_KEY."
    exit 1
fi

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

echo "=== WebSocket API endpoints ==="

run ws-logon           ws-logon
run ws-account-status  ws-account-status

# Place a test order (will likely be rejected on testnet without balance, but captures the flow)
run ws-order-place     ws-order-place "$SYMBOL" BUY LIMIT -q 0.001 -p 10000 || true

echo "=== Done ==="
echo "Output in: $OUT"
