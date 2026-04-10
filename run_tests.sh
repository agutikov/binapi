#!/usr/bin/env bash
set -euo pipefail

ctest --test-dir _build --output-on-failure -j20
