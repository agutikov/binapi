// SPDX-License-Identifier: Apache-2.0
//
// binapi2 stream_buffer and stream_recorder benchmarks.
//
// Measures throughput of all 3 buffer types x 4 consumer APIs (12 combinations)
// plus stream_recorder with callback_sink.

#include <binapi2/fapi/detail/hopping_stream_buffer.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace binapi2::fapi;
using clock_t = std::chrono::high_resolution_clock;

// -- Harness --

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

// Payload: ~100 byte JSON-like string
static const std::string payload =
    R"({"e":"bookTicker","u":123,"s":"BTCUSDT","b":"71000.00","B":"1.0","a":"71001.00","A":"2.0"})";

constexpr std::size_t N = 10000;

// ===================================================================
// stream_buffer (single-executor)
// ===================================================================

boost::cobalt::task<bench_result>
bench_single_drain()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            co_await boost::cobalt::join(
                producer(), buf.async_drain([](const std::string&) {}));
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        co_await boost::cobalt::join(
            producer(), buf.async_drain([](const std::string&) {}));
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("single/drain", rounds * N, elapsed);
}

boost::cobalt::task<bench_result>
bench_single_read()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            auto consumer = [&]() -> boost::cobalt::task<void> {
                while (true) {
                    auto r = co_await buf.async_read();
                    if (!r) break;
                }
            };
            co_await boost::cobalt::join(producer(), consumer());
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        auto consumer = [&]() -> boost::cobalt::task<void> {
            while (true) {
                auto r = co_await buf.async_read();
                if (!r) break;
            }
        };
        co_await boost::cobalt::join(producer(), consumer());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("single/read", rounds * N, elapsed);
}

boost::cobalt::task<bench_result>
bench_single_batch()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            auto consumer = [&]() -> boost::cobalt::task<void> {
                while (true) {
                    auto r = co_await buf.async_read_batch(64);
                    if (!r) break;
                }
            };
            co_await boost::cobalt::join(producer(), consumer());
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        auto consumer = [&]() -> boost::cobalt::task<void> {
            while (true) {
                auto r = co_await buf.async_read_batch(64);
                if (!r) break;
            }
        };
        co_await boost::cobalt::join(producer(), consumer());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("single/batch", rounds * N, elapsed);
}

boost::cobalt::task<bench_result>
bench_single_reader()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            auto consumer = [&]() -> boost::cobalt::task<void> {
                while (true) {
                    auto rd = buf.reader();
                    auto rv = co_await boost::cobalt::as_result(rd);
                    if (!rv) break;
                }
            };
            co_await boost::cobalt::join(producer(), consumer());
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        auto consumer = [&]() -> boost::cobalt::task<void> {
            while (true) {
                auto rd = buf.reader();
                auto rv = co_await boost::cobalt::as_result(rd);
                if (!rv) break;
            }
        };
        co_await boost::cobalt::join(producer(), consumer());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("single/reader", rounds * N, elapsed);
}

// ===================================================================
// hopping_stream_buffer (cross-executor, tested same-executor here)
// ===================================================================

boost::cobalt::task<bench_result>
bench_hopping_drain()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::hopping_stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            co_await boost::cobalt::join(
                producer(), buf.async_drain([](const std::string&) {}));
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::hopping_stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        co_await boost::cobalt::join(
            producer(), buf.async_drain([](const std::string&) {}));
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("hopping/drain", rounds * N, elapsed);
}

boost::cobalt::task<bench_result>
bench_hopping_read()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::hopping_stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            auto consumer = [&]() -> boost::cobalt::task<void> {
                while (true) {
                    auto r = co_await buf.async_read();
                    if (!r) break;
                }
            };
            co_await boost::cobalt::join(producer(), consumer());
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::hopping_stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        auto consumer = [&]() -> boost::cobalt::task<void> {
            while (true) {
                auto r = co_await buf.async_read();
                if (!r) break;
            }
        };
        co_await boost::cobalt::join(producer(), consumer());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("hopping/read", rounds * N, elapsed);
}

boost::cobalt::task<bench_result>
bench_hopping_batch()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::hopping_stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            auto consumer = [&]() -> boost::cobalt::task<void> {
                while (true) {
                    auto r = co_await buf.async_read_batch(64);
                    if (!r) break;
                }
            };
            co_await boost::cobalt::join(producer(), consumer());
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::hopping_stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        auto consumer = [&]() -> boost::cobalt::task<void> {
            while (true) {
                auto r = co_await buf.async_read_batch(64);
                if (!r) break;
            }
        };
        co_await boost::cobalt::join(producer(), consumer());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("hopping/batch", rounds * N, elapsed);
}

boost::cobalt::task<bench_result>
bench_hopping_reader()
{
    std::size_t rounds = 4;
    for (;;) {
        auto t0 = clock_t::now();
        for (std::size_t r = 0; r < rounds; ++r) {
            detail::hopping_stream_buffer<std::string> buf(1024);
            auto producer = [&]() -> boost::cobalt::task<void> {
                for (std::size_t i = 0; i < N; ++i)
                    co_await buf.async_push(std::string(payload));
                buf.close();
            };
            auto consumer = [&]() -> boost::cobalt::task<void> {
                while (true) {
                    auto rd = buf.reader();
                    auto rv = co_await boost::cobalt::as_result(rd);
                    if (!rv) break;
                }
            };
            co_await boost::cobalt::join(producer(), consumer());
        }
        if (clock_t::now() - t0 >= std::chrono::seconds(1)) break;
        rounds *= 2;
    }

    auto t0 = clock_t::now();
    for (std::size_t r = 0; r < rounds; ++r) {
        detail::hopping_stream_buffer<std::string> buf(1024);
        auto producer = [&]() -> boost::cobalt::task<void> {
            for (std::size_t i = 0; i < N; ++i)
                co_await buf.async_push(std::string(payload));
            buf.close();
        };
        auto consumer = [&]() -> boost::cobalt::task<void> {
            while (true) {
                auto rd = buf.reader();
                auto rv = co_await boost::cobalt::as_result(rd);
                if (!rv) break;
            }
        };
        co_await boost::cobalt::join(producer(), consumer());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();
    co_return compute("hopping/reader", rounds * N, elapsed);
}

// ===================================================================
// hopping_stream_buffer — cross-thread
// ===================================================================

// Buffer created on co_main's executor. Producer runs on a separate io_thread,
// calling async_push which hops to co_main's executor, writes, hops back.
// Consumer (drain) runs on co_main. Single pass.
constexpr std::size_t HOP_N = N * 10;

// Helper: producer runs on io_thread, pushes via async_push (hops cross-thread).
void hopping_push_thread(detail::hopping_stream_buffer<std::string>& buf, std::size_t n)
{
    detail::io_thread io;
    io.run_sync([&]() -> boost::cobalt::task<void> {
        for (std::size_t i = 0; i < n; ++i)
            co_await buf.async_push(std::string(payload));
        co_await buf.async_close();
    }());
}

boost::cobalt::task<bench_result>
bench_hopping_cross_drain()
{
    detail::hopping_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { hopping_push_thread(buf, HOP_N); });

    auto t0 = clock_t::now();
    co_await buf.async_drain([](const std::string&) {});
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("hopping_cross/drain", HOP_N, elapsed);
}

boost::cobalt::task<bench_result>
bench_hopping_cross_read()
{
    detail::hopping_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { hopping_push_thread(buf, HOP_N); });

    auto t0 = clock_t::now();
    while (true) {
        auto r = co_await buf.async_read();
        if (!r) break;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("hopping_cross/read", HOP_N, elapsed);
}

boost::cobalt::task<bench_result>
bench_hopping_cross_batch()
{
    detail::hopping_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { hopping_push_thread(buf, HOP_N); });

    auto t0 = clock_t::now();
    while (true) {
        auto r = co_await buf.async_read_batch(64);
        if (!r) break;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("hopping_cross/batch", HOP_N, elapsed);
}

boost::cobalt::task<bench_result>
bench_hopping_cross_reader()
{
    detail::hopping_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { hopping_push_thread(buf, HOP_N); });

    auto t0 = clock_t::now();
    while (true) {
        auto rd = buf.reader();
        auto rv = co_await boost::cobalt::as_result(rd);
        if (!rv) break;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("hopping_cross/reader", HOP_N, elapsed);
}

// ===================================================================
// threadsafe_stream_buffer (SPSC, cross-thread)
// ===================================================================

// Helper: push N items from a std::thread, spin-retry on full.
void ts_push_thread(detail::threadsafe_stream_buffer<std::string>& buf, std::size_t n)
{
    for (std::size_t i = 0; i < n; ++i)
        while (!buf.push(std::string(payload))) {}
    buf.close();
}

// Threadsafe benchmarks use a single large pass — creating/destroying
// threadsafe_stream_buffer repeatedly in a coroutine causes PMR lifetime
// issues with cobalt::channel. Using 10x N for a meaningful measurement.
constexpr std::size_t TS_N = N * 100;

boost::cobalt::task<bench_result>
bench_threadsafe_drain()
{
    detail::threadsafe_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { ts_push_thread(buf, TS_N); });

    auto t0 = clock_t::now();
    co_await boost::cobalt::join(
        buf.async_forward(),
        buf.async_drain([](const std::string&) {}));
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("threadsafe/drain", TS_N, elapsed);
}

boost::cobalt::task<bench_result>
bench_threadsafe_read()
{
    detail::threadsafe_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { ts_push_thread(buf, TS_N); });

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read();
            if (!r) break;
        }
    };

    auto t0 = clock_t::now();
    co_await boost::cobalt::join(buf.async_forward(), consumer());
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("threadsafe/read", TS_N, elapsed);
}

boost::cobalt::task<bench_result>
bench_threadsafe_batch()
{
    detail::threadsafe_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { ts_push_thread(buf, TS_N); });

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read_batch(64);
            if (!r) break;
        }
    };

    auto t0 = clock_t::now();
    co_await boost::cobalt::join(buf.async_forward(), consumer());
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("threadsafe/batch", TS_N, elapsed);
}

boost::cobalt::task<bench_result>
bench_threadsafe_reader()
{
    detail::threadsafe_stream_buffer<std::string> buf(1024);
    std::thread producer([&] { ts_push_thread(buf, TS_N); });

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto rd = buf.reader();
            auto rv = co_await boost::cobalt::as_result(rd);
            if (!rv) break;
        }
    };

    auto t0 = clock_t::now();
    co_await boost::cobalt::join(buf.async_forward(), consumer());
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() - t0).count();

    producer.join();
    co_return compute("threadsafe/reader", TS_N, elapsed);
}

// ===================================================================
// Main
// ===================================================================

boost::cobalt::task<void> run_benchmarks()
{
    std::vector<bench_result> results;
    bench_result r{};

    std::printf("== stream_buffer (single-executor), %zu msgs/round ==\n\n", N);
    print_header();

    r = co_await bench_single_drain();   results.push_back(r); print_result(r);
    r = co_await bench_single_read();    results.push_back(r); print_result(r);
    r = co_await bench_single_batch();   results.push_back(r); print_result(r);
    r = co_await bench_single_reader();  results.push_back(r); print_result(r);

    std::printf("\n== hopping_stream_buffer (same-executor path), %zu msgs/round ==\n\n", N);
    print_header();

    r = co_await bench_hopping_drain();  results.push_back(r); print_result(r);
    r = co_await bench_hopping_read();   results.push_back(r); print_result(r);
    r = co_await bench_hopping_batch();  results.push_back(r); print_result(r);
    r = co_await bench_hopping_reader(); results.push_back(r); print_result(r);

    std::printf("\n== hopping_stream_buffer (cross-thread), %zu msgs ==\n\n", HOP_N);
    print_header();

    r = co_await bench_hopping_cross_drain();  results.push_back(r); print_result(r);
    r = co_await bench_hopping_cross_read();   results.push_back(r); print_result(r);
    r = co_await bench_hopping_cross_batch();  results.push_back(r); print_result(r);
    r = co_await bench_hopping_cross_reader(); results.push_back(r); print_result(r);

    // Note: threadsafe benchmarks run a single measured pass (not calibrated)
    // because creating/destroying threadsafe_stream_buffer in a loop inside
    // a coroutine causes PMR lifetime issues with repeated cobalt channel allocation.
    std::printf("\n== threadsafe_stream_buffer (SPSC cross-thread), %zu msgs ==\n\n", N);
    print_header();

    r = co_await bench_threadsafe_drain();  results.push_back(r); print_result(r);
    r = co_await bench_threadsafe_read();   results.push_back(r); print_result(r);
    r = co_await bench_threadsafe_batch();  results.push_back(r); print_result(r);
    r = co_await bench_threadsafe_reader(); results.push_back(r); print_result(r);

    // Summary
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
