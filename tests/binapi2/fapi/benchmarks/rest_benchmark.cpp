// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures REST pipeline benchmarks.

/// @file rest_benchmark.cpp
/// @brief Throughput benchmarks for the full REST pipeline against the Postman mock server.
///
/// Measures the complete request cycle: request struct → endpoint_traits → query serialization
/// → HMAC signing → HTTP dispatch → JSON deserialization → result<Response>.
///
/// Prerequisites:
///   1. scripts/api/postman_mock/start.sh   (generates certs, starts docker compose)
///   2. Set SSL_CERT_FILE env var to the mock server's CA cert.
///
/// Run:
///   SSL_CERT_FILE=compose/postman-mock/certs/server.crt
///       ./_build/tests/binapi2/fapi/benchmarks/rest_benchmark

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <boost/cobalt/main.hpp>
#include <boost/cobalt/task.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

using namespace binapi2::fapi;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string env_or(const char* name, const char* fallback)
{
    const char* v = std::getenv(name);
    return v ? v : fallback;
}

// ---------------------------------------------------------------------------
// Harness
// ---------------------------------------------------------------------------

struct bench_result
{
    const char* name;
    std::size_t iterations;
    double ns_per_op;
    double ops_per_sec;
};

void print_header()
{
    std::printf("%-35s %10s %12s %12s\n",
                "Benchmark", "iters", "us/op", "ops/sec");
    std::printf("%-35s %10s %12s %12s\n",
                std::string(35, '-').c_str(), "----------",
                "------------", "------------");
}

void print_result(const bench_result& r)
{
    std::printf("%-35s %10zu %12.0f %12.0f\n",
                r.name, r.iterations, r.ns_per_op / 1000.0, r.ops_per_sec);
}

/// @brief Run an async benchmark: calibrate for ~1s then measure.
template<typename Fn>
boost::cobalt::task<bench_result>
run_bench(const char* name, Fn&& fn)
{
    using clock = std::chrono::high_resolution_clock;

    // Warmup
    for (int i = 0; i < 3; ++i) co_await fn();

    // Calibrate
    std::size_t iters = 10;
    for (;;) {
        auto t0 = clock::now();
        for (std::size_t i = 0; i < iters; ++i) co_await fn();
        if (clock::now() - t0 >= std::chrono::seconds(1)) break;
        iters *= 2;
    }

    // Measure
    auto t0 = clock::now();
    for (std::size_t i = 0; i < iters; ++i) co_await fn();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - t0).count();

    double ns_op = static_cast<double>(elapsed_ns) / static_cast<double>(iters);
    double ops = 1e9 / ns_op;

    co_return bench_result{name, iters, ns_op, ops};
}

// ---------------------------------------------------------------------------
// Validation: verify mock server is reachable and endpoints return valid data
// ---------------------------------------------------------------------------

boost::cobalt::task<bool> validate(client& c)
{
    std::printf("== Validating mock server ==\n\n");
    int failures = 0;

    auto ping = co_await c.market_data.async_execute(types::ping_request_t{});
    if (!ping) {
        std::fprintf(stderr, "  FAIL ping: %s\n", ping.err.message.c_str());
        ++failures;
    } else {
        std::printf("  OK   ping\n");
    }

    auto time = co_await c.market_data.async_execute(types::server_time_request_t{});
    if (!time) {
        std::fprintf(stderr, "  FAIL server_time: %s\n", time.err.message.c_str());
        ++failures;
    } else {
        std::printf("  OK   server_time\n");
    }

    auto depth = co_await c.market_data.async_execute(
        types::order_book_request_t{.symbol = "BTCUSDT", .limit = 5});
    if (!depth) {
        std::fprintf(stderr, "  FAIL order_book: %s\n", depth.err.message.c_str());
        ++failures;
    } else {
        std::printf("  OK   order_book\n");
    }

    auto acct = co_await c.account.async_execute(types::account_information_request_t{});
    if (!acct) {
        std::fprintf(stderr, "  FAIL account: %s\n", acct.err.message.c_str());
        ++failures;
    } else {
        std::printf("  OK   account (signed)\n");
    }

    if (failures > 0) {
        std::fprintf(stderr, "\n%d endpoint(s) failed — is the mock server running?\n", failures);
        std::fprintf(stderr, "Start it with: scripts/api/postman_mock/start.sh\n");
        co_return false;
    }

    std::printf("\nAll endpoints OK.\n");
    co_return true;
}

// ---------------------------------------------------------------------------
// Benchmarks
// ---------------------------------------------------------------------------

boost::cobalt::task<void> run_benchmarks(client& c)
{
    std::vector<bench_result> results;
    bench_result r{};

    // ── Public market data (unsigned) ──

    std::printf("\n== Public market data (unsigned) ==\n\n");
    print_header();

    r = co_await run_bench("ping", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(types::ping_request_t{});
        if (!res) std::fprintf(stderr, "ping failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("server_time", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(types::server_time_request_t{});
        if (!res) std::fprintf(stderr, "server_time failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("order_book (depth 5)", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(
            types::order_book_request_t{.symbol = "BTCUSDT", .limit = 5});
        if (!res) std::fprintf(stderr, "order_book failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("recent_trades", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(
            types::recent_trades_request_t{.symbol = "BTCUSDT"});
        if (!res) std::fprintf(stderr, "recent_trades failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("klines (1h)", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(
            types::klines_request_t{.symbol = "BTCUSDT", .interval = types::kline_interval_t::h1});
        if (!res) std::fprintf(stderr, "klines failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("price_ticker", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(
            types::price_ticker_request_t{.symbol = "BTCUSDT"});
        if (!res) std::fprintf(stderr, "price_ticker failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("book_ticker", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(
            types::book_ticker_request_t{.symbol = "BTCUSDT"});
        if (!res) std::fprintf(stderr, "book_ticker failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("mark_price", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(
            types::mark_price_request_t{.symbol = "BTCUSDT"});
        if (!res) std::fprintf(stderr, "mark_price failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("exchange_info", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.market_data.async_execute(types::exchange_info_request_t{});
        if (!res) std::fprintf(stderr, "exchange_info failed\n");
    });
    results.push_back(r); print_result(r);

    // ── Signed endpoints ──

    std::printf("\n== Signed endpoints (HMAC + auth header) ==\n\n");
    print_header();

    r = co_await run_bench("account_information", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.account.async_execute(types::account_information_request_t{});
        if (!res) std::fprintf(stderr, "account failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("balances", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.account.async_execute(types::balances_request_t{});
        if (!res) std::fprintf(stderr, "balances failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("position_risk", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.account.async_execute(types::position_risk_request_t{});
        if (!res) std::fprintf(stderr, "position_risk failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("query_order", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.trade.async_execute(
            types::query_order_request_t{.symbol = "BTCUSDT", .orderId = 22542179});
        if (!res) std::fprintf(stderr, "query_order failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("all_open_orders", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.trade.async_execute(types::all_open_orders_request_t{});
        if (!res) std::fprintf(stderr, "all_open_orders failed\n");
    });
    results.push_back(r); print_result(r);

    // ── User data streams ──

    std::printf("\n== User data streams (API key auth) ==\n\n");
    print_header();

    r = co_await run_bench("start_listen_key", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.user_data_streams.async_execute(types::start_listen_key_request_t{});
        if (!res) std::fprintf(stderr, "start_listen_key failed\n");
    });
    results.push_back(r); print_result(r);

    r = co_await run_bench("keepalive_listen_key", [&]() -> boost::cobalt::task<void> {
        auto res = co_await c.user_data_streams.async_execute(types::keepalive_listen_key_request_t{});
        if (!res) std::fprintf(stderr, "keepalive_listen_key failed\n");
    });
    results.push_back(r); print_result(r);

    // ── Summary ──

    std::printf("\n== Summary ==\n\n");

    double min_us = results[0].ns_per_op / 1000.0;
    double max_us = results[0].ns_per_op / 1000.0;
    for (const auto& br : results) {
        double us = br.ns_per_op / 1000.0;
        min_us = std::min(min_us, us);
        max_us = std::max(max_us, us);
    }

    std::printf("Fastest: %.0f us/op  (%.0f ops/sec)\n", min_us, 1e6 / min_us);
    std::printf("Slowest: %.0f us/op  (%.0f ops/sec)\n", max_us, 1e6 / max_us);
    std::printf("Total benchmarks: %zu\n", results.size());
    std::printf("\nNote: includes TLS handshake + TCP connect per request (no connection pooling).\n");
}

} // namespace

boost::cobalt::main co_main(int, char*[])
{
    const auto host = env_or("MOCK_HOST", "localhost");
    const auto port = env_or("MOCK_PORT", "8443");

    std::printf("REST benchmark against mock server %s:%s\n\n", host.c_str(), port.c_str());

    config cfg;
    cfg.rest_host = host;
    cfg.rest_port = port;
    cfg.api_key = "test-api-key";
    cfg.secret_key = "test-secret-key";
    cfg.ca_cert_file = env_or("SSL_CERT_FILE", "");

    client c(cfg);

    if (!co_await validate(c))
        co_return 1;

    co_await run_benchmarks(c);

    co_return 0;
}
