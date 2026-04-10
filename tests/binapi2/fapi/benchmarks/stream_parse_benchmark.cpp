// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures stream parsing benchmarks.

/// @file stream_parse_benchmark.cpp
/// @brief Throughput benchmarks for market and user stream event JSON parsing.
///
/// Uses basic_market_stream and basic_user_stream with a replay transport
/// that feeds pre-recorded JSON without any I/O.

#include "replay_transport.hpp"

#include <binapi2/fapi/streams/combined_market_stream.hpp>
#include <binapi2/fapi/streams/dynamic_market_stream.hpp>
#include <binapi2/fapi/streams/market_stream.hpp>
#include <binapi2/fapi/streams/user_stream.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/cobalt/main.hpp>
#include <boost/cobalt/task.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace binapi2::fapi;
using replay = benchmarks::replay_transport;
using market_stream_replay = streams::basic_market_stream<replay>;
using user_stream_replay = streams::basic_user_stream<replay>;

// ---------------------------------------------------------------------------
// Harness
// ---------------------------------------------------------------------------

struct bench_result
{
    const char* name;
    std::size_t iterations;
    std::size_t bytes;
    double ns_per_op;
    double ops_per_sec;
    double mb_per_sec;
};

void print_header()
{
    std::printf("%-42s %10s %10s %12s %10s\n",
                "Benchmark", "iters", "ns/op", "ops/sec", "MB/s");
    std::printf("%-42s %10s %10s %12s %10s\n",
                std::string(42, '-').c_str(), "----------", "----------",
                "------------", "----------");
}

void print_result(const bench_result& r)
{
    std::printf("%-42s %10zu %10.0f %12.0f %10.1f\n",
                r.name, r.iterations, r.ns_per_op, r.ops_per_sec, r.mb_per_sec);
}

bench_result compute(const char* name, std::size_t bytes_per_op,
                     std::size_t total_ops, long long elapsed_ns)
{
    double ns_op = static_cast<double>(elapsed_ns) / static_cast<double>(total_ops);
    double ops = 1e9 / ns_op;
    double mbps = (static_cast<double>(bytes_per_op) * ops) / (1024.0 * 1024.0);
    return {name, total_ops, bytes_per_op, ns_op, ops, mbps};
}

// ---------------------------------------------------------------------------
// Fixture loading
// ---------------------------------------------------------------------------

std::string read_file(const std::string& path)
{
    std::ifstream f(path);
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", path.c_str());
        std::exit(1);
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}


// ---------------------------------------------------------------------------
// Upfront fixture validation
// ---------------------------------------------------------------------------

template<typename T>
bool validate_parse(const char* name, const std::string& json)
{
    if (json.empty()) {
        std::fprintf(stderr, "  FAIL %-30s empty input\n", name);
        return false;
    }
    T value{};
    glz::context ctx{};
    auto ec = glz::read<binapi2::fapi::detail::json_read_opts>(value, json, ctx);
    if (ec) {
        std::fprintf(stderr, "  FAIL %-30s %s\n", name, glz::format_error(ec, json).c_str());
        return false;
    }
    std::printf("  OK   %-30s %zu bytes\n", name, json.size());
    return true;
}

// ---------------------------------------------------------------------------
// Market stream benchmarks
// ---------------------------------------------------------------------------

template<class Subscription>
boost::cobalt::task<bench_result>
bench_market_subscribe(const char* name, const std::string& json,
                       const Subscription& sub, std::size_t n)
{
    using clock = std::chrono::high_resolution_clock;

    auto cfg = config::testnet_config();
    market_stream_replay ms(cfg);
    ms.set_max_reconnects(0);
    ms.transport().messages.assign(n, json);

    // Warmup
    ms.transport().reset();
    {
        auto gen = ms.subscribe(sub);
        while (gen) { auto ev = co_await gen; if (!ev) break; }
    }

    // Calibrate
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            ms.transport().reset();
            auto gen = ms.subscribe(sub);
            while (gen) { auto ev = co_await gen; if (!ev) break; }
        }
        if (clock::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    // Measure
    auto t0 = clock::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        ms.transport().reset();
        auto gen = ms.subscribe(sub);
        while (gen) { auto ev = co_await gen; if (!ev) break; }
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - t0).count();

    co_return compute(name, json.size(), rounds * n, elapsed);
}

// ---------------------------------------------------------------------------
// User stream benchmarks
// ---------------------------------------------------------------------------

boost::cobalt::task<bench_result>
bench_user_subscribe(const char* name, const std::string& json, std::size_t n)
{
    using clock = std::chrono::high_resolution_clock;

    auto cfg = config::testnet_config();
    user_stream_replay us(cfg);
    us.set_max_reconnects(0);
    us.transport().messages.assign(n, json);

    // Warmup
    us.transport().reset();
    {
        auto gen = us.subscribe("fake-listen-key");
        while (gen) { auto ev = co_await gen; if (!ev) break; }
    }

    // Calibrate
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            us.transport().reset();
            auto gen = us.subscribe("fake-listen-key");
            while (gen) { auto ev = co_await gen; if (!ev) break; }
        }
        if (clock::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    // Measure
    auto t0 = clock::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        us.transport().reset();
        auto gen = us.subscribe("fake-listen-key");
        while (gen) { auto ev = co_await gen; if (!ev) break; }
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - t0).count();

    co_return compute(name, json.size(), rounds * n, elapsed);
}

// ---------------------------------------------------------------------------
// Top-level coroutine
// ---------------------------------------------------------------------------

boost::cobalt::task<void> run_benchmarks()
{
    using namespace binapi2::fapi::types;

    const std::string fixtures = FIXTURES_PATH;

    constexpr std::size_t N = 1000;

    // -- Load fixtures --
    const auto book_ticker_json = read_file(fixtures + "/book_ticker.json");
    const auto ticker_json = read_file(fixtures + "/ticker.json");
    const auto mark_price_json = read_file(fixtures + "/mark_price.json");
    const auto agg_trade_json = read_file(fixtures + "/aggregate_trade.json");
    const auto depth_json = read_file(fixtures + "/depth.json");
    const auto kline_json = read_file(fixtures + "/kline.json");
    const auto liquidation_json = read_file(fixtures + "/liquidation.json");
    const auto account_update_json = read_file(fixtures + "/account_update.json");
    const auto order_trade_json = read_file(fixtures + "/order_trade_update.json");
    const auto margin_call_json = read_file(fixtures + "/margin_call.json");

    // ── Validate all fixtures before benchmarking ──

    std::printf("== Validating fixtures ==\n\n");
    int failures = 0;

    // Market events
    if (!validate_parse<book_ticker_stream_event_t>("book_ticker", book_ticker_json)) ++failures;
    if (!validate_parse<ticker_stream_event_t>("ticker_24hr", ticker_json)) ++failures;
    if (!validate_parse<mark_price_stream_event_t>("mark_price", mark_price_json)) ++failures;
    if (!validate_parse<aggregate_trade_stream_event_t>("aggregate_trade", agg_trade_json)) ++failures;
    if (!validate_parse<depth_stream_event_t>("depth", depth_json)) ++failures;
    if (!validate_parse<kline_stream_event_t>("kline", kline_json)) ++failures;
    if (!validate_parse<liquidation_order_stream_event_t>("liquidation", liquidation_json)) ++failures;

    // User events
    if (!validate_parse<account_update_event_t>("account_update", account_update_json)) ++failures;
    if (!validate_parse<order_trade_update_event_t>("order_trade_update", order_trade_json)) ++failures;
    if (!validate_parse<margin_call_event_t>("margin_call", margin_call_json)) ++failures;

    if (failures > 0) {
        std::fprintf(stderr, "\n%d fixture(s) failed validation — aborting.\n", failures);
        co_return;
    }
    std::printf("\nAll fixtures OK.\n");

    std::vector<bench_result> results;
    bench_result r{};

    // ── Market stream: subscribe() generator ──

    std::printf("\n== Market stream: subscribe() generator, %zu msgs/round ==\n\n", N);
    print_header();

    r = co_await bench_market_subscribe("subscribe/book_ticker", book_ticker_json,
        book_ticker_subscription{.symbol = "BTCUSDT"}, N);
    results.push_back(r); print_result(r);

    r = co_await bench_market_subscribe("subscribe/ticker_24hr", ticker_json,
        ticker_subscription{.symbol = "BTCUSDT"}, N);
    results.push_back(r); print_result(r);

    r = co_await bench_market_subscribe("subscribe/mark_price", mark_price_json,
        mark_price_subscription{.symbol = "BTCUSDT"}, N);
    results.push_back(r); print_result(r);

    r = co_await bench_market_subscribe("subscribe/kline", kline_json,
        kline_subscription{.symbol = "BTCUSDT", .interval = kline_interval_t::m5}, N);
    results.push_back(r); print_result(r);

    r = co_await bench_market_subscribe("subscribe/depth", depth_json,
        diff_book_depth_subscription{.symbol = "BTCUSDT", .speed = "100ms"}, N);
    results.push_back(r); print_result(r);

    r = co_await bench_market_subscribe("subscribe/aggregate_trade", agg_trade_json,
        aggregate_trade_subscription{.symbol = "BTCUSDT"}, N);
    results.push_back(r); print_result(r);

    r = co_await bench_market_subscribe("subscribe/liquidation", liquidation_json,
        liquidation_order_subscription{.symbol = "BTCUSDT"}, N);
    results.push_back(r); print_result(r);

    // ── User stream: subscribe() dispatch ──

    std::printf("\n== User stream: subscribe() dispatch, %zu msgs/round ==\n\n", N);
    print_header();

    r = co_await bench_user_subscribe("subscribe/account_update", account_update_json, N);
    results.push_back(r); print_result(r);

    r = co_await bench_user_subscribe("subscribe/order_trade_update", order_trade_json, N);
    results.push_back(r); print_result(r);

    r = co_await bench_user_subscribe("subscribe/margin_call", margin_call_json, N);
    results.push_back(r); print_result(r);

    // ── Summary ──

    std::printf("\n== Summary ==\n\n");

    double min_ns = results[0].ns_per_op;
    double max_ns = results[0].ns_per_op;
    for (const auto& br : results) {
        min_ns = std::min(min_ns, br.ns_per_op);
        max_ns = std::max(max_ns, br.ns_per_op);
    }

    std::printf("Fastest: %.0f ns/op  (%.0f ops/sec)\n", min_ns, 1e9 / min_ns);
    std::printf("Slowest: %.0f ns/op  (%.0f ops/sec)\n", max_ns, 1e9 / max_ns);
    std::printf("Total benchmarks: %zu\n", results.size());
}

} // namespace

boost::cobalt::main co_main(int, char*[])
{
    co_await run_benchmarks();
    co_return 0;
}
