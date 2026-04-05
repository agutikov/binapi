#!/usr/bin/env bash
#
# Stop the Postman mock server.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
COMPOSE_DIR="$REPO_ROOT/compose/postman-mock"

echo "Stopping Postman mock server..."
docker compose -f "$COMPOSE_DIR/docker-compose.yml" down

echo "Mock server stopped."
