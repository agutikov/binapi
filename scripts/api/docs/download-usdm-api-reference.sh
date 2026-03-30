#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd -- "${SCRIPT_DIR}/../../../.." && pwd)"
OUTPUT_DIR="${PROJECT_DIR}/docs/api/binance/usds-margined-futures"
BASE_URL="https://developers.binance.com/docs/derivatives/usds-margined-futures/general-info"
DOC_PREFIX="https://developers.binance.com/docs/derivatives/usds-margined-futures/"

mkdir -p "${OUTPUT_DIR}"

printf 'Downloading Binance USD-M Futures REST and WebSocket API documentation...\n'
printf 'Source: %s\n' "${BASE_URL}"
printf 'Output: %s\n' "${OUTPUT_DIR}"

wget \
  --recursive \
  --level=inf \
  --no-clobber \
  --page-requisites \
  --convert-links \
  --adjust-extension \
  --span-hosts=off \
  --domains=developers.binance.com \
  --no-parent \
  --directory-prefix="${OUTPUT_DIR}" \
  --execute robots=off \
  --accept-regex='developers\.binance\.com/docs/derivatives/usds-margined-futures/' \
  "${BASE_URL}"

printf 'Download complete. Offline mirror saved under: %s\n' "${OUTPUT_DIR}"
printf 'Open the mirrored entry page from: %s\n' \
  "${OUTPUT_DIR}/developers.binance.com/docs/derivatives/usds-margined-futures/general-info.html"
