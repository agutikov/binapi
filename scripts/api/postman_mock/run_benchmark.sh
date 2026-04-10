#!/usr/bin/env bash
#
# Run the REST benchmark against the Postman mock server.
# Assumes the mock server is already running (scripts/api/postman_mock/start.sh).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
CERT="$REPO_ROOT/compose/postman-mock/certs/server.crt"
BIN="$REPO_ROOT/_build/tests/binapi2/fapi/benchmarks/rest_benchmark"

if [[ ! -f "$BIN" ]]; then
    echo "error: benchmark binary not found: $BIN" >&2
    echo "Run build.sh first." >&2
    exit 1
fi

if [[ ! -f "$CERT" ]]; then
    echo "error: TLS certificate not found: $CERT" >&2
    echo "Run scripts/api/postman_mock/start.sh first." >&2
    exit 1
fi


SSL_CERT_FILE="$CERT" exec "$BIN" "$@"
