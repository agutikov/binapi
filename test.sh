#!/usr/bin/env bash
set -euo pipefail

cmake -B _build -DBINAPI2_WITH_TESTS=ON
cmake --build _build --target decimal_test -j"$(nproc)"
ctest --test-dir _build --output-on-failure
