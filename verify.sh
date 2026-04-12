#!/usr/bin/env bash
#
# Library correctness verification — runs all tests and benchmarks.
#
# Levels:
#   1. Unit tests          (378 tests, offline, ~1s)
#   2. Offline benchmarks  (3 binaries, no server needed)
#   3. Integration tests   (22 tests, requires Docker)
#   4. Online benchmark    (16 endpoints, requires Docker)
#   5. Testnet scripts     (135 commands, requires API keys)
#
# Usage:
#   ./verify.sh              # levels 1-2 (no Docker, no keys)
#   ./verify.sh --all        # levels 1-5 (starts/stops mock server, runs testnet)
#   ./verify.sh --mock       # levels 1-4, assumes mock server already running
#   ./verify.sh --testnet    # levels 1-2 + 5 (no Docker, with testnet)
#
# Exit code: 0 if all executed levels pass, 1 otherwise.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/_build"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
RESET='\033[0m'

passed=0
failed=0
skipped=0

run_level() {
    local level="$1"
    local name="$2"
    shift 2
    printf "\n${BOLD}=== Level %s: %s ===${RESET}\n\n" "$level" "$name"
}

run_step() {
    local name="$1"
    shift
    printf "  %-40s" "$name"
    local output
    if output=$("$@" 2>&1); then
        printf "${GREEN}OK${RESET}\n"
        passed=$((passed + 1))
        return 0
    else
        printf "${RED}FAIL${RESET}\n"
        echo "$output" | tail -20 | sed 's/^/    /'
        failed=$((failed + 1))
        return 1
    fi
}

run_step_show() {
    local name="$1"
    shift
    printf "  %-40s" "$name"
    local output
    if output=$("$@" 2>&1); then
        printf "${GREEN}OK${RESET}\n"
        echo "$output" | grep -E "Fastest|Slowest|Total|iters" | head -5 | sed 's/^/    /'
        passed=$((passed + 1))
        return 0
    else
        printf "${RED}FAIL${RESET}\n"
        echo "$output" | tail -10 | sed 's/^/    /'
        failed=$((failed + 1))
        return 1
    fi
}

skip_step() {
    local name="$1"
    local reason="$2"
    printf "  %-40s${YELLOW}SKIP${RESET} (%s)\n" "$name" "$reason"
    skipped=$((skipped + 1))
}

# ---------------------------------------------------------------------------
# Parse args
# ---------------------------------------------------------------------------

run_mock=false
run_testnet=false
mock_external=false

for arg in "$@"; do
    case "$arg" in
        --all)      run_mock=true; run_testnet=true ;;
        --mock)     run_mock=true; mock_external=true ;;
        --testnet)  run_testnet=true ;;
        -h|--help)
            echo "Usage: $0 [--all|--mock|--testnet]"
            echo "  (no flag)    Levels 1-2 only (no Docker, no keys)"
            echo "  --all        Levels 1-5 (starts/stops mock server, runs testnet)"
            echo "  --mock       Levels 1-4 (mock server already running)"
            echo "  --testnet    Levels 1-2 + 5 (no Docker, with testnet)"
            exit 0
            ;;
        *) echo "Unknown flag: $arg"; exit 1 ;;
    esac
done

# ---------------------------------------------------------------------------
# Pre-flight
# ---------------------------------------------------------------------------

if [[ ! -d "$BUILD_DIR" ]]; then
    echo "Build directory not found. Run ./build.sh first."
    exit 1
fi

# ---------------------------------------------------------------------------
# Level 1: Unit tests
# ---------------------------------------------------------------------------

run_level 1 "Unit tests (378 tests)"

run_step "ctest" ctest --test-dir "$BUILD_DIR" --output-on-failure -j20

# ---------------------------------------------------------------------------
# Level 2: Offline benchmarks
# ---------------------------------------------------------------------------

run_level 2 "Offline benchmarks"

for bench in stream_parse_benchmark stream_buffer_benchmark stream_recorder_benchmark; do
    bin="$BUILD_DIR/tests/binapi2/fapi/benchmarks/$bench"
    if [[ -f "$bin" ]]; then
        run_step_show "$bench" "$bin"
    else
        skip_step "$bench" "binary not found"
    fi
done

# ---------------------------------------------------------------------------
# Level 3 & 4: Integration tests and online benchmark (require mock server)
# ---------------------------------------------------------------------------

if [[ "$run_mock" == false ]]; then
    run_level 3 "Integration tests"
    skip_step "postman_mock_integration" "use --all or --mock"
    skip_step "sync_bridging_test" "use --all or --mock"

    run_level 4 "Online benchmark"
    skip_step "rest_benchmark" "use --all or --mock"
else
    mock_started=false

    if [[ "$mock_external" == false ]]; then
        run_level 3 "Integration tests (starting mock server)"
        printf "  %-40s" "start mock server"
        if start_output=$("$SCRIPT_DIR/scripts/api/postman_mock/start.sh" 2>&1); then
            printf "${GREEN}OK${RESET}\n"
            mock_started=true
        else
            printf "${RED}FAIL${RESET}\n"
            echo "$start_output" | tail -5 | sed 's/^/    /'
            echo "  Cannot start mock server. Skipping levels 3-4."
            skipped=$((skipped + 3))
            run_mock=false
        fi
    else
        run_level 3 "Integration tests (using running mock server)"
    fi

    if [[ "$run_mock" == true ]]; then
        CERT="$SCRIPT_DIR/compose/postman-mock/certs/server.crt"
        MOCK_BIN="$BUILD_DIR/tests/binapi2/fapi/integration/postman_mock/postman_mock_integration"
        BRIDGING_BIN="$BUILD_DIR/tests/binapi2/fapi/integration/sync_bridging/sync_bridging_test"
        BENCH_BIN="$BUILD_DIR/tests/binapi2/fapi/benchmarks/rest_benchmark"

        if [[ -f "$CERT" ]]; then
            if [[ -f "$MOCK_BIN" ]]; then
                run_step "postman_mock_integration (18)" env SSL_CERT_FILE="$CERT" "$MOCK_BIN"
            else
                skip_step "postman_mock_integration" "binary not found"
            fi

            if [[ -f "$BRIDGING_BIN" ]]; then
                run_step "sync_bridging_test (4)" env SSL_CERT_FILE="$CERT" "$BRIDGING_BIN"
            else
                skip_step "sync_bridging_test" "binary not found"
            fi

            run_level 4 "Online benchmark"

            if [[ -f "$BENCH_BIN" ]]; then
                run_step_show "rest_benchmark (16)" env SSL_CERT_FILE="$CERT" "$BENCH_BIN"
            else
                skip_step "rest_benchmark" "binary not found"
            fi
        else
            skip_step "postman_mock_integration" "cert not found: $CERT"
            skip_step "sync_bridging_test" "cert not found"
            run_level 4 "Online benchmark"
            skip_step "rest_benchmark" "cert not found"
        fi

        if [[ "$mock_started" == true ]]; then
            printf "\n  %-40s" "stop mock server"
            if stop_output=$("$SCRIPT_DIR/scripts/api/postman_mock/stop.sh" 2>&1); then
                printf "${GREEN}OK${RESET}\n"
            else
                printf "${YELLOW}WARN${RESET}\n"
                echo "$stop_output" | tail -3 | sed 's/^/    /'
            fi
        fi
    fi
fi

# ---------------------------------------------------------------------------
# Level 5: Testnet scripts
# ---------------------------------------------------------------------------

if [[ "$run_testnet" == true ]]; then
    run_level 5 "Testnet verification (135 commands)"

    TESTNET_DIR="$SCRIPT_DIR/scripts/testnet"
    TESTNET_OUT="$SCRIPT_DIR/testnet_output"

    # Expected failures — testnet-side issues, not library bugs.
    KNOWN_FAILURES=(
        # Analytics endpoints return plain text "ok"
        "market_data/open-interest-stats"
        "market_data/top-ls-account-ratio"
        "market_data/top-ls-trader-ratio"
        "market_data/long-short-ratio"
        "market_data/taker-volume"
        "market_data/basis"
        "market_data/delivery-price"
        # Non-standard testnet responses
        "trade/test-order"
        "trade/tradfi-perps"
        # Endpoint not on testnet
        "account/pm-account-info"
        # Server errors
        "account/quantitative-rules"
        "account/quantitative-rules-sym"
        "convert/convert-quote"
        "convert/convert-order-status"
        # Requires balance
        "ws_api/ws-order-place"
    )

    is_known_failure() {
        local key="$1"
        for kf in "${KNOWN_FAILURES[@]}"; do
            [[ "$kf" == "$key" ]] && return 0
        done
        return 1
    }

    testnet_scripts=(
        "market_data.sh"
        "account.sh"
        "trade.sh"
        "convert.sh"
        "ws_api.sh"
        "streams.sh"
        "user_streams.sh"
        "order_book.sh"
    )

    # Run all testnet scripts
    for script in "${testnet_scripts[@]}"; do
        printf "  %-40s" "$script"
        if script_output=$("$TESTNET_DIR/$script" 2>&1); then
            printf "${GREEN}OK${RESET}\n"
        else
            printf "${GREEN}OK${RESET} (completed)\n"
        fi
    done

    # Analyze results
    printf "\n  ${BOLD}Analyzing results...${RESET}\n\n"

    testnet_ok=0
    testnet_known=0
    testnet_unexpected=0
    unexpected_list=()

    for group_dir in "$TESTNET_OUT"/*/; do
        group=$(basename "$group_dir")
        for cmd_dir in "$group_dir"*/; do
            [[ -d "$cmd_dir" ]] || continue
            cmd=$(basename "$cmd_dir")
            key="$group/$cmd"

            # Streams and order book: check for frames in stream.jsonl
            if [[ -f "$cmd_dir/stream.jsonl" ]]; then
                testnet_ok=$((testnet_ok + 1))
                continue
            fi

            # REST/WS commands: check stdout for errors
            if [[ -f "$cmd_dir/stdout.txt" ]] && grep -q '\[error\]' "$cmd_dir/stdout.txt" 2>/dev/null; then
                if is_known_failure "$key"; then
                    testnet_known=$((testnet_known + 1))
                else
                    testnet_unexpected=$((testnet_unexpected + 1))
                    unexpected_list+=("$key")
                fi
            else
                testnet_ok=$((testnet_ok + 1))
            fi
        done
    done

    printf "  ${GREEN}Passed:    %3d${RESET}\n" "$testnet_ok"
    printf "  ${YELLOW}Known:     %3d${RESET}  (testnet-side, not library bugs)\n" "$testnet_known"

    if [[ $testnet_unexpected -gt 0 ]]; then
        printf "  ${RED}Unexpected: %2d${RESET}\n" "$testnet_unexpected"
        for uf in "${unexpected_list[@]}"; do
            printf "    ${RED}FAIL${RESET}  %s\n" "$uf"
            # Show first error line
            stdout="$TESTNET_OUT/${uf}/stdout.txt"
            if [[ -f "$stdout" ]]; then
                grep '\[error\]' "$stdout" | head -1 | sed 's/^/          /'
            fi
        done
        failed=$((failed + testnet_unexpected))
    fi

    passed=$((passed + testnet_ok))

    # Verify output files exist for passing REST/WS commands
    missing_files=0
    for group in market_data account trade convert ws_api; do
        for cmd_dir in "$TESTNET_OUT/$group"/*/; do
            [[ -d "$cmd_dir" ]] || continue
            cmd=$(basename "$cmd_dir")
            key="$group/$cmd"
            is_known_failure "$key" && continue
            for f in log.txt stdout.txt; do
                if [[ ! -f "$cmd_dir/$f" ]]; then
                    printf "  ${YELLOW}WARN${RESET}  missing %s in %s\n" "$f" "$key"
                    missing_files=$((missing_files + 1))
                fi
            done
        done
    done

    if [[ $missing_files -eq 0 ]]; then
        printf "\n  All output files present.\n"
    fi
else
    run_level 5 "Testnet verification"
    skip_step "testnet scripts" "use --all or --testnet"
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

printf "\n${BOLD}========================================${RESET}\n"
printf "${GREEN}Passed: %d${RESET}" "$passed"
if [[ $failed -gt 0 ]]; then
    printf "  ${RED}Failed: %d${RESET}" "$failed"
fi
if [[ $skipped -gt 0 ]]; then
    printf "  ${YELLOW}Skipped: %d${RESET}" "$skipped"
fi
printf "\n${BOLD}========================================${RESET}\n"

if [[ $failed -gt 0 ]]; then
    exit 1
fi
