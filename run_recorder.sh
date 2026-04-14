#!/usr/bin/env bash
#
# Run async-recorder against Binance production with every supported
# feed enabled:
#
#   - all-market screener feeds (bookTicker, markPriceArr, tickerArr)
#   - selector driven by rolling TF aggregates (selector.yaml)
#   - per-symbol detail monitor: aggTrade + bookTicker + markPrice@1s
#     + forceOrder + full diff depth + periodic REST depth snapshots
#   - REST periodic sync: fundingRate, klines_1m, openInterestHist,
#     longShortRatio
#
# Storage warning: --full-depth produces 20-80 GB/day per major symbol
# per the plan. Point --root at a volume with enough space or drop
# --full-depth / --with-depth for a lighter run.
#
# Arguments after `--` are forwarded to the binary so you can override
# flags without touching this script, e.g.:
#   ./run_recorder.sh -- --depth-levels 10 --stats-seconds 5

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$REPO/_build/examples/binapi2/fapi/async-recorder/binapi2-fapi-async-recorder"

if [[ ! -x "$BIN" ]]; then
    echo "error: recorder not built. Run ./build.sh first." >&2
    exit 1
fi

TS="$(date -u +%Y%m%dT%H%M%SZ)"
ROOT="${ASYNC_RECORDER_ROOT:-$REPO/data/run-$TS}"
LOG="$ROOT/recorder.log"

mkdir -p "$ROOT"

echo "async-recorder (live, max data):"
echo "  root   = $ROOT"
echo "  log    = $LOG"
echo "  binary = $BIN"
echo

exec "$BIN" \
    --live \
    --root "$ROOT" \
    --logfile "$LOG" \
    --loglevel trace \
    --selector "$REPO/examples/binapi2/fapi/async-recorder/selector.yaml" \
    --stats-seconds 5 \
    --with-depth \
    --full-depth \
    --depth-resnap-seconds 600 \
    "$@"
