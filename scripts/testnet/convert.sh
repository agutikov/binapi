#!/usr/bin/env bash
# Run Convert REST demo-cli commands against Binance testnet.
# Saves request, response, and log for each command.
#
# Note: Convert endpoints may not be available on testnet — FAIL is expected.
#
# Usage: scripts/testnet/convert.sh [output_dir]
#   output_dir defaults to testnet_output/convert

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CLI="$ROOT_DIR/_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli"

OUT="${1:-$ROOT_DIR/testnet_output/convert}"
mkdir -p "$OUT"

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

echo "=== Convert ==="

run convert-quote        convert-quote BTC USDT 0.001 || true
run convert-order-status convert-order-status 0 || true

echo "=== Done ==="
echo "Output in: $OUT"
