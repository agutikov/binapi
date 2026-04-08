#!/usr/bin/env bash
#
# Stop the Postman mock server.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
COMPOSE_DIR="$REPO_ROOT/compose/postman-mock"

CERT_DIR="$COMPOSE_DIR/certs"
WORKDIR="$COMPOSE_DIR/workdir"

echo "Stopping Postman mock server..."
docker compose -f "$COMPOSE_DIR/docker-compose.yml" down

rm -rf "$CERT_DIR" "$WORKDIR"

echo "Mock server stopped."
