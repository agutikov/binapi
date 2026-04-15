#!/usr/bin/env bash

set -euo pipefail

#./utils/format.py -q -j10

cmake \
    -S . \
    -B _build \
    -DCMAKE_BUILD_TYPE=Debug


cmake --build _build -j10
