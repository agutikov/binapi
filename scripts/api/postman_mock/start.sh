#!/usr/bin/env bash
#
# Start the Postman mock server.
#
# 1. Generates TLS certs (if missing)
# 2. Merges mock response examples into the Postman collection
# 3. Starts the mock server via docker compose
#
# Usage:
#   scripts/api/postman_mock/start.sh
#   MOCK_PORT=9443 scripts/api/postman_mock/start.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
COMPOSE_DIR="$REPO_ROOT/compose/postman-mock"
CERT_DIR="$COMPOSE_DIR/certs"
WORKDIR="$COMPOSE_DIR/workdir"

COLLECTION="$REPO_ROOT/docs/api/binance-api-postman/collections/Binance Derivatives Trading USDS Futures API.json"
RESPONSES_DIR="$COMPOSE_DIR/responses"
MAPPING="$COMPOSE_DIR/response_mapping.json"

# --- Generate self-signed TLS certificate if missing ---

if [[ ! -f "$CERT_DIR/server.crt" || ! -f "$CERT_DIR/server.key" ]]; then
    echo "Generating self-signed TLS certificate..."
    mkdir -p "$CERT_DIR"
    openssl req -x509 -newkey rsa:2048 \
        -keyout "$CERT_DIR/server.key" \
        -out "$CERT_DIR/server.crt" \
        -days 365 -nodes \
        -subj '/CN=localhost' \
        -addext 'basicConstraints=critical,CA:TRUE' \
        -addext 'subjectAltName=DNS:localhost,IP:127.0.0.1' \
        2>/dev/null
    echo "Certificate: $CERT_DIR/server.crt"
fi

# --- Merge responses into collection ---

echo "Preparing enriched collection..."
mkdir -p "$WORKDIR"

python3 "$SCRIPT_DIR/merge_responses.py" \
    --collection "$COLLECTION" \
    --responses "$RESPONSES_DIR" \
    --mapping "$MAPPING" \
    --output "$WORKDIR/collection.json"

# --- Start the mock server ---

export MOCK_PORT="${MOCK_PORT:-8443}"

echo "Starting Postman mock server on port $MOCK_PORT..."
docker compose -f "$COMPOSE_DIR/docker-compose.yml" up -d --build --wait

echo ""
echo "Mock server ready at https://localhost:$MOCK_PORT"
echo "CA certificate:    $CERT_DIR/server.crt"
echo ""
echo "Run integration test:"
echo "  SSL_CERT_FILE=$CERT_DIR/server.crt \\"
echo "      ./_build/tests/binapi2/fapi/integration/postman_mock/postman_mock_integration"
