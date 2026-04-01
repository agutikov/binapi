#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

doxygen Doxyfile

echo "Documentation generated in _docs/html/index.html"
