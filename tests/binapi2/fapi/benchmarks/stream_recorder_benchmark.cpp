// SPDX-License-Identifier: Apache-2.0
//
// binapi2 stream_recorder benchmarks — callback_sink and spdlog_sink.
// file_sink requires BOOST_ASIO_HAS_IO_URING and is not benchmarked here.

#include <binapi2/fapi/streams/sinks/callback_sink.hpp>
#include <binapi2/fapi/streams/sinks/file_sink.hpp>
#include <binapi2/fapi/streams/sinks/spdlog_sink.hpp>
#include <binapi2/fapi/streams/stream_recorder.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace {

using namespace binapi2::fapi;
using clock_t = std::chrono::high_resolution_clock;

struct bench_result
{
    const char* name;
    std::size_t iterations;
    double ns_per_op;
    double ops_per_sec;
};

void print_header()
{
    std::printf("%-50s %10s %10s %12s\n", "Benchmark", "iters", "ns/op", "ops/sec");
    std::printf("%-50s %10s %10s %12s\n",
                std::string(50, '-').c_str(), "----------", "----------", "------------");
}

void print_result(const bench_result& r)
{
    std::printf("%-50s %10zu %10.0f %12.0f\n",
                r.name, r.iterations, r.ns_per_op, r.ops_per_sec);
}

bench_result compute(const char* name, std::size_t total_ops, long long elapsed_ns)
{
    double ns_op = static_cast<double>(elapsed_ns) / static_cast<double>(total_ops);
    return {name, total_ops, ns_op, 1e9 / ns_op};
}

static const std::string payload =
    R"({"e":"bookTicker","u":123,"s":"BTCUSDT","b":"71000.00","B":"1.0","a":"71001.00","A":"2.0"})";

constexpr std::size_t N = 100000;

// ===================================================================
// callback_sink
// ===================================================================

bench_result bench_callback_sink()
{
    // Warmup
    {
        streams::stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(
            streams::sinks::callback_sink([](const std::string&) {}));
        recorder.start();
        for (std::size_t i = 0; i < 1000; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }

    // Measure
    auto t0 = clock_t::now();
    {
        streams::stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(
            streams::sinks::callback_sink([](const std::string&) {}));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    return compute("recorder/callback_sink", N, elapsed);
}

// ===================================================================
// callback_sink — multiple streams
// ===================================================================

bench_result bench_callback_sink_multi()
{
    auto t0 = clock_t::now();
    {
        streams::stream_recorder recorder(1024);
        auto& buf_a = recorder.add_stream(
            streams::sinks::callback_sink([](const std::string&) {}));
        auto& buf_b = recorder.add_stream(
            streams::sinks::callback_sink([](const std::string&) {}));
        recorder.start();
        for (std::size_t i = 0; i < N / 2; ++i) {
            buf_a.push(std::string(payload));
            buf_b.push(std::string(payload));
        }
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    return compute("recorder/callback_sink_multi(2)", N, elapsed);
}

// ===================================================================
// spdlog_sink (null sink — measures recorder overhead, not disk I/O)
// ===================================================================

bench_result bench_spdlog_sink()
{
    spdlog::init_thread_pool(8192, 1);
    auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto logger = std::make_shared<spdlog::async_logger>(
        "bench_rec", null_sink, spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::info);

    // Warmup
    {
        streams::spdlog_stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < 1000; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }

    // Measure
    auto t0 = clock_t::now();
    {
        streams::spdlog_stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    spdlog::drop("bench_rec");
    return compute("recorder/spdlog_sink", N, elapsed);
}

// ===================================================================
// file_sink (async via asio::stream_file + io_uring)
// ===================================================================

bench_result bench_async_file_sink()
{
    const std::string path = "/tmp/binapi2_bench_async_file.jsonl";

    // Warmup
    {
        streams::file_stream_recorder recorder(4096);
        auto& buf = recorder.add_stream(
            streams::sinks::file_sink(recorder.io_context(), path));
        recorder.start();
        for (std::size_t i = 0; i < 1000; ++i)
            while (!buf.push(std::string(payload))) {}
        recorder.stop();
    }

    // Measure
    auto t0 = clock_t::now();
    {
        streams::file_stream_recorder recorder(4096);
        auto& buf = recorder.add_stream(
            streams::sinks::file_sink(recorder.io_context(), path));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            while (!buf.push(std::string(payload))) {}
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    std::filesystem::remove(path);
    return compute("recorder/file_sink(io_uring)", N, elapsed);
}

// ===================================================================
// spdlog_sink writing to a real file
// ===================================================================

bench_result bench_spdlog_file_sink()
{
    const std::string path = "/tmp/binapi2_bench_spdlog.jsonl";

    spdlog::init_thread_pool(8192, 1);
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
    file_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::async_logger>(
        "bench_rec_file", file_sink, spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::info);

    // Warmup
    {
        streams::spdlog_stream_recorder recorder(4096);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < 1000; ++i)
            while (!buf.push(std::string(payload))) {}
        recorder.stop();
    }

    // Measure
    auto t0 = clock_t::now();
    {
        streams::spdlog_stream_recorder recorder(4096);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            while (!buf.push(std::string(payload))) {}
        recorder.stop();
    }
    logger->flush();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    spdlog::drop("bench_rec_file");
    std::filesystem::remove(path);
    return compute("recorder/spdlog_sink(file)", N, elapsed);
}

} // namespace

int main()
{
    std::vector<bench_result> results;

    std::printf("== stream_recorder benchmarks, %zu msgs ==\n\n", N);
    print_header();

    auto r = bench_callback_sink();       results.push_back(r); print_result(r);
    r = bench_callback_sink_multi();      results.push_back(r); print_result(r);
    r = bench_spdlog_sink();              results.push_back(r); print_result(r);
    r = bench_async_file_sink();          results.push_back(r); print_result(r);
    r = bench_spdlog_file_sink();         results.push_back(r); print_result(r);

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
    return 0;
}
