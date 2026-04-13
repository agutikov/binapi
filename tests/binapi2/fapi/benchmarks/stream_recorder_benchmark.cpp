// SPDX-License-Identifier: Apache-2.0
//
// binapi2 stream_recorder benchmarks — callback_sink and spdlog_sink.
// file_sink requires BOOST_ASIO_HAS_IO_URING and is not benchmarked here.

#include <binapi2/fapi/streams/detail/sinks/callback_sink.hpp>
#include <binapi2/fapi/streams/detail/sinks/file_sink.hpp>
#include <binapi2/fapi/streams/detail/sinks/spdlog_sink.hpp>
#include <binapi2/fapi/streams/detail/async_stream_recorder.hpp>
#include <binapi2/fapi/streams/detail/threaded_stream_recorder.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

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
        streams::threaded_stream_recorder recorder(1024);
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
        streams::threaded_stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(
            streams::sinks::callback_sink([](const std::string&) {}));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    return compute("threaded/callback_sink", N, elapsed);
}

// ===================================================================
// callback_sink — multiple streams
// ===================================================================

bench_result bench_callback_sink_multi()
{
    auto t0 = clock_t::now();
    {
        streams::threaded_stream_recorder recorder(1024);
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
    return compute("threaded/callback_sink_multi(2)", N, elapsed);
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
        streams::threaded_spdlog_stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < 1000; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }

    // Measure
    auto t0 = clock_t::now();
    {
        streams::threaded_spdlog_stream_recorder recorder(1024);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            buf.push(std::string(payload));
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    spdlog::drop("bench_rec");
    return compute("threaded/spdlog_sink", N, elapsed);
}

// ===================================================================
// file_sink (async via asio::stream_file + io_uring)
// ===================================================================

bench_result bench_async_file_sink()
{
    const std::string path = "/tmp/binapi2_bench_async_file.jsonl";

    // Warmup
    {
        streams::threaded_file_stream_recorder recorder(4096);
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
        streams::threaded_file_stream_recorder recorder(4096);
        auto& buf = recorder.add_stream(
            streams::sinks::file_sink(recorder.io_context(), path));
        recorder.start();
        for (std::size_t i = 0; i < N; ++i)
            while (!buf.push(std::string(payload))) {}
        recorder.stop();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    std::filesystem::remove(path);
    return compute("threaded/file_sink(io_uring)", N, elapsed);
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
        streams::threaded_spdlog_stream_recorder recorder(4096);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        for (std::size_t i = 0; i < 1000; ++i)
            while (!buf.push(std::string(payload))) {}
        recorder.stop();
    }

    // Measure
    auto t0 = clock_t::now();
    {
        streams::threaded_spdlog_stream_recorder recorder(4096);
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
    return compute("threaded/spdlog_sink(file)", N, elapsed);
}

// ===================================================================
// async_stream_recorder — single-executor variant
// ===================================================================

boost::cobalt::task<void>
run_async_callback(std::size_t n)
{
    streams::async_stream_recorder recorder(1024);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([](const std::string&) {}));

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (std::size_t i = 0; i < n; ++i)
            co_await buf.async_push(std::string(payload));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
}

bench_result bench_async_callback_sink()
{
    // Warmup
    boost::cobalt::run(run_async_callback(1000));

    auto t0 = clock_t::now();
    boost::cobalt::run(run_async_callback(N));
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       clock_t::now() - t0).count();
    return compute("async/callback_sink", N, elapsed);
}

boost::cobalt::task<void>
run_async_spdlog(std::shared_ptr<spdlog::logger> logger, std::size_t n)
{
    streams::async_spdlog_stream_recorder recorder(1024);
    auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (std::size_t i = 0; i < n; ++i)
            co_await buf.async_push(std::string(payload));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
}

bench_result bench_async_spdlog_sink()
{
    spdlog::init_thread_pool(8192, 1);
    auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto logger = std::make_shared<spdlog::async_logger>(
        "bench_async_rec", null_sink, spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::info);

    boost::cobalt::run(run_async_spdlog(logger, 1000));

    auto t0 = clock_t::now();
    boost::cobalt::run(run_async_spdlog(logger, N));
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       clock_t::now() - t0).count();

    spdlog::drop("bench_async_rec");
    return compute("async/spdlog_sink", N, elapsed);
}

boost::cobalt::task<void>
run_async_file(const std::string& path, std::size_t n)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ctx = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    streams::async_file_stream_recorder recorder(4096);
    auto& buf = recorder.add_stream(streams::sinks::file_sink(ctx, path));

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (std::size_t i = 0; i < n; ++i)
            co_await buf.async_push(std::string(payload));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
}

bench_result bench_async_file_sink_colocated()
{
    const std::string path = "/tmp/binapi2_bench_async_file_colo.jsonl";

    boost::cobalt::run(run_async_file(path, 1000)); // warmup

    auto t0 = clock_t::now();
    boost::cobalt::run(run_async_file(path, N));
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       clock_t::now() - t0).count();

    auto sz = std::filesystem::file_size(path);
    std::filesystem::remove(path);
    if (sz < N) {
        std::fprintf(stderr,
                     "async/file_sink bench wrote only %zu bytes (expected >= %zu)\n",
                     static_cast<std::size_t>(sz), N);
    }
    return compute("async/file_sink(io_uring)", N, elapsed);
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
    r = bench_async_callback_sink();      results.push_back(r); print_result(r);
    r = bench_async_spdlog_sink();        results.push_back(r); print_result(r);
    r = bench_async_file_sink_colocated(); results.push_back(r); print_result(r);

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
