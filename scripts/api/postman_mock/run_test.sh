#!/usr/bin/env bash
#
# Run the integration test against the Postman mock server.
# Assumes the mock server is already running (scripts/api/postman_mock/start.sh).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
CERT="$REPO_ROOT/compose/postman-mock/certs/server.crt"
BIN_MOCK="$REPO_ROOT/_build/tests/binapi2/fapi/integration/postman_mock/postman_mock_integration"
BIN_BRIDGING="$REPO_ROOT/_build/tests/binapi2/fapi/integration/sync_bridging/sync_bridging_test"

for bin in "$BIN_MOCK" "$BIN_BRIDGING"; do
    if [[ ! -f "$bin" ]]; then
        echo "error: test binary not found: $bin" >&2
        echo "Run build.sh first." >&2
        exit 1
    fi
done

if [[ ! -f "$CERT" ]]; then
    echo "error: TLS certificate not found: $CERT" >&2
    echo "Run scripts/api/postman_mock/start.sh first." >&2
    exit 1
fi

LOG_DIR="$REPO_ROOT/testnet_output/postman_mock"
mkdir -p "$LOG_DIR"

rc=0

echo "=== Postman mock integration test ==="
SSL_CERT_FILE="$CERT" "$BIN_MOCK" "$@" 2>&1 | tee "$LOG_DIR/run_test.log"
rc=$((rc | ${PIPESTATUS[0]}))

echo ""
echo "=== Sync bridging test ==="
SSL_CERT_FILE="$CERT" "$BIN_BRIDGING" 2>&1 | tee "$LOG_DIR/sync_bridging.log"
rc=$((rc | ${PIPESTATUS[0]}))

exit $rc
