// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures stream parsing benchmarks.

/// @file stream_parse_benchmark.cpp
/// @brief Throughput benchmarks for market and user stream event JSON parsing.
///
/// Measures raw glz::read performance for each event type without transport.
/// Fixtures are either recorded testnet samples or synthetic JSON matching
/// the Binance wire format.

#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/user_stream_events.hpp>

#include <glaze/glaze.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


namespace {

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

/// Run `fn` repeatedly for at least `min_time` and return stats.
template<typename Fn>
bench_result run_bench(const char* name, std::size_t bytes_per_op, Fn&& fn)
{
    using clock = std::chrono::high_resolution_clock;

    // Warmup
    for (int i = 0; i < 100; ++i) fn();

    // Calibrate: find iteration count that fills ~1 second
    const auto min_ns = std::chrono::nanoseconds(std::chrono::seconds(1));
    std::size_t iters = 1000;
    for (;;) {
        auto t0 = clock::now();
        for (std::size_t i = 0; i < iters; ++i) fn();
        auto elapsed = clock::now() - t0;
        if (elapsed >= min_ns) break;
        iters *= 2;
    }

    // Measure
    auto t0 = clock::now();
    for (std::size_t i = 0; i < iters; ++i) fn();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - t0).count();

    double ns_op = static_cast<double>(elapsed_ns) / static_cast<double>(iters);
    double ops = 1e9 / ns_op;
    double mbps = (static_cast<double>(bytes_per_op) * ops) / (1024.0 * 1024.0);

    return {name, iters, bytes_per_op, ns_op, ops, mbps};
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

std::string read_first_line(const std::string& path)
{
    std::ifstream f(path);
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", path.c_str());
        std::exit(1);
    }
    std::string line;
    std::getline(f, line);
    return line;
}

std::vector<std::string> read_lines(const std::string& path)
{
    std::ifstream f(path);
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", path.c_str());
        std::exit(1);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty()) lines.push_back(std::move(line));
    }
    return lines;
}

// ---------------------------------------------------------------------------
// Parse helpers (same as production code, without transport)
// ---------------------------------------------------------------------------

template<typename Event>
bool parse_event(const std::string& json, Event& out)
{
    glz::context ctx{};
    return !glz::read<binapi2::fapi::detail::json_read_opts>(out, json, ctx);
}

// ---------------------------------------------------------------------------
// Market stream benchmarks
// ---------------------------------------------------------------------------

template<typename Event>
bench_result bench_market_event(const char* name, const std::string& json)
{
    return run_bench(name, json.size(), [&] {
        Event event{};
        if (!parse_event(json, event)) {
            std::fprintf(stderr, "FATAL: parse failed for %s\n", name);
            std::exit(1);
        }
    });
}

// ---------------------------------------------------------------------------
// User stream dispatch — mirrors production logic from user_streams.cpp
// ---------------------------------------------------------------------------

bool match_event(const std::string& payload, const char* event_name)
{
    std::string with_space = std::string("\"e\": \"") + event_name + "\"";
    std::string without_space = std::string("\"e\":\"") + event_name + "\"";
    return payload.find(with_space) != std::string::npos
        || payload.find(without_space) != std::string::npos;
}

template<typename Event>
bool try_parse_user(const std::string& payload, const char* event_name,
                    binapi2::fapi::types::user_stream_event_t& out)
{
    if (!match_event(payload, event_name)) return false;
    Event event{};
    glz::context ctx{};
    if (glz::read<binapi2::fapi::detail::json_read_opts>(event, payload, ctx)) return false;
    out = std::move(event);
    return true;
}

binapi2::fapi::result<binapi2::fapi::types::user_stream_event_t>
dispatch_user_event(const std::string& payload)
{
    using namespace binapi2::fapi::types;
    using R = binapi2::fapi::result<user_stream_event_t>;

    user_stream_event_t event;

    if (try_parse_user<account_update_event_t>(payload, "ACCOUNT_UPDATE", event)) return R::success(std::move(event));
    if (try_parse_user<order_trade_update_event_t>(payload, "ORDER_TRADE_UPDATE", event)) return R::success(std::move(event));
    if (try_parse_user<margin_call_event_t>(payload, "MARGIN_CALL", event)) return R::success(std::move(event));
    if (try_parse_user<listen_key_expired_event_t>(payload, "listenKeyExpired", event)) return R::success(std::move(event));
    if (try_parse_user<account_config_update_event_t>(payload, "ACCOUNT_CONFIG_UPDATE", event)) return R::success(std::move(event));
    if (try_parse_user<trade_lite_event_t>(payload, "TRADE_LITE", event)) return R::success(std::move(event));
    if (try_parse_user<algo_order_update_event_t>(payload, "ALGO_UPDATE", event)) return R::success(std::move(event));
    if (try_parse_user<conditional_order_trigger_reject_event_t>(payload, "CONDITIONAL_ORDER_TRIGGER_REJECT", event)) return R::success(std::move(event));
    if (try_parse_user<grid_update_event_t>(payload, "GRID_UPDATE", event)) return R::success(std::move(event));
    if (try_parse_user<strategy_update_event_t>(payload, "STRATEGY_UPDATE", event)) return R::success(std::move(event));

    return R::failure({binapi2::fapi::error_code::json, 0, 0, "unknown user stream event type", payload});
}

bench_result bench_user_dispatch(const char* name, const std::string& json)
{
    return run_bench(name, json.size(), [&] {
        auto r = dispatch_user_event(json);
        if (!r) {
            std::fprintf(stderr, "FATAL: user dispatch failed for %s: %s\n",
                         name, r.err.message.c_str());
            std::exit(1);
        }
    });
}

// ---------------------------------------------------------------------------
// Batch (array) parse benchmark
// ---------------------------------------------------------------------------

template<typename ArrayEvent>
bench_result bench_array_event(const char* name, const std::string& json)
{
    return run_bench(name, json.size(), [&] {
        ArrayEvent events{};
        glz::context ctx{};
        if (glz::read<binapi2::fapi::detail::json_read_opts>(events, json, ctx)) {
            std::fprintf(stderr, "FATAL: array parse failed for %s\n", name);
            std::exit(1);
        }
    });
}

} // namespace

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    using namespace binapi2::fapi::types;

    const std::string fixtures = FIXTURES_PATH;
    const std::string testnet = TESTNET_STREAMS_PATH;

    // -- Load fixtures --

    // Market events: prefer testnet recordings, fall back to synthetic fixtures
    const auto book_ticker_json = read_first_line(testnet + "/stream-book-ticker/stream.jsonl");
    const auto ticker_json = read_first_line(testnet + "/stream-ticker/stream.jsonl");
    const auto mark_price_json = read_first_line(testnet + "/stream-mark-price/stream.jsonl");
    const auto agg_trade_json = read_file(fixtures + "/aggregate_trade.json");
    const auto depth_json = read_file(fixtures + "/depth.json");
    const auto kline_json = read_file(fixtures + "/kline.json");
    const auto liquidation_json = read_file(fixtures + "/liquidation.json");

    // User events (synthetic)
    const auto account_update_json = read_file(fixtures + "/account_update.json");
    const auto order_trade_json = read_file(fixtures + "/order_trade_update.json");
    const auto margin_call_json = read_file(fixtures + "/margin_call.json");

    // Array events (testnet recordings — large payloads)
    const auto all_mini_tickers_lines = read_lines(testnet + "/stream-all-mini-tickers/stream.jsonl");
    const auto all_tickers_lines = read_lines(testnet + "/stream-all-tickers/stream.jsonl");

    std::vector<bench_result> results;

    // ── Market stream: single-event parsing ──

    print_header();
    std::printf("\n== Market stream events ==\n\n");
    print_header();

    results.push_back(bench_market_event<book_ticker_stream_event_t>("book_ticker", book_ticker_json));
    print_result(results.back());

    results.push_back(bench_market_event<ticker_stream_event_t>("ticker_24hr", ticker_json));
    print_result(results.back());

    results.push_back(bench_market_event<mark_price_stream_event_t>("mark_price", mark_price_json));
    print_result(results.back());

    results.push_back(bench_market_event<aggregate_trade_stream_event_t>("aggregate_trade", agg_trade_json));
    print_result(results.back());

    results.push_back(bench_market_event<depth_stream_event_t>("depth (5 levels)", depth_json));
    print_result(results.back());

    results.push_back(bench_market_event<kline_stream_event_t>("kline", kline_json));
    print_result(results.back());

    results.push_back(bench_market_event<liquidation_order_stream_event_t>("liquidation", liquidation_json));
    print_result(results.back());

    // ── Market stream: array/batch parsing ──

    std::printf("\n== Market stream batch events ==\n\n");
    print_header();

    if (!all_mini_tickers_lines.empty()) {
        results.push_back(bench_array_event<all_market_mini_ticker_stream_event>(
            "all_mini_tickers (batch)", all_mini_tickers_lines[0]));
        print_result(results.back());
    }

    if (!all_tickers_lines.empty()) {
        results.push_back(bench_array_event<all_market_ticker_stream_event>(
            "all_tickers (batch)", all_tickers_lines[0]));
        print_result(results.back());
    }

    // ── User stream: direct parsing per type ──

    std::printf("\n== User stream events (direct parse) ==\n\n");
    print_header();

    results.push_back(bench_market_event<account_update_event_t>("account_update", account_update_json));
    print_result(results.back());

    results.push_back(bench_market_event<order_trade_update_event_t>("order_trade_update", order_trade_json));
    print_result(results.back());

    results.push_back(bench_market_event<margin_call_event_t>("margin_call", margin_call_json));
    print_result(results.back());

    // ── User stream: full dispatch (match_event + try_parse chain) ──

    std::printf("\n== User stream dispatch (match + parse) ==\n\n");
    print_header();

    results.push_back(bench_user_dispatch("dispatch/account_update", account_update_json));
    print_result(results.back());

    results.push_back(bench_user_dispatch("dispatch/order_trade_update", order_trade_json));
    print_result(results.back());

    results.push_back(bench_user_dispatch("dispatch/margin_call", margin_call_json));
    print_result(results.back());

    // ── Summary ──

    std::printf("\n== Summary ==\n\n");

    double min_ns = results[0].ns_per_op;
    double max_ns = results[0].ns_per_op;
    for (const auto& r : results) {
        min_ns = std::min(min_ns, r.ns_per_op);
        max_ns = std::max(max_ns, r.ns_per_op);
    }

    std::printf("Fastest: %.0f ns/op  (%.0f ops/sec)\n", min_ns, 1e9 / min_ns);
    std::printf("Slowest: %.0f ns/op  (%.0f ops/sec)\n", max_ns, 1e9 / max_ns);
    std::printf("Total benchmarks: %zu\n", results.size());

    return 0;
}
