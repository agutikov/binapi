// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for stream_buffer variants (3 buffers x 4 APIs = 12 combinations).

#include <binapi2/fapi/detail/hopping_stream_buffer.hpp>
#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <string>
#include <thread>
#include <vector>

using namespace binapi2::fapi;

template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

// ===================================================================
// stream_buffer (single-executor) x 4 APIs
// ===================================================================

// -- async_drain --

static boost::cobalt::task<std::vector<std::string>>
do_single_drain()
{
    detail::stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("a"));
        co_await buf.async_push(std::string("b"));
        co_await buf.async_push(std::string("c"));
        buf.close();
    };

    co_await boost::cobalt::join(
        producer(),
        buf.async_drain([&out](const std::string& s) { out.push_back(s); }));
    co_return out;
}

TEST(SingleBuffer, Drain)
{
    auto out = run_sync(do_single_drain());
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "a");
    EXPECT_EQ(out[1], "b");
    EXPECT_EQ(out[2], "c");
}

// -- async_read --

static boost::cobalt::task<std::vector<std::string>>
do_single_read()
{
    detail::stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("x"));
        co_await buf.async_push(std::string("y"));
        buf.close();
    };

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read();
            if (!r) break;
            out.push_back(*r);
        }
    };

    co_await boost::cobalt::join(producer(), consumer());
    co_return out;
}

TEST(SingleBuffer, Read)
{
    auto out = run_sync(do_single_read());
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "x");
    EXPECT_EQ(out[1], "y");
}

// -- async_read_batch --

static boost::cobalt::task<std::vector<std::string>>
do_single_batch()
{
    detail::stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (int i = 0; i < 5; ++i)
            co_await buf.async_push(std::to_string(i));
        buf.close();
    };

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read_batch(3);
            if (!r) break;
            for (auto& s : *r) out.push_back(std::move(s));
        }
    };

    co_await boost::cobalt::join(producer(), consumer());
    co_return out;
}

TEST(SingleBuffer, Batch)
{
    auto out = run_sync(do_single_batch());
    ASSERT_EQ(out.size(), 5u);
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(out[i], std::to_string(i));
}

// -- reader (via async_drain which uses reader internally — test close behavior) --

static boost::cobalt::task<int>
do_single_backpressure()
{
    detail::stream_buffer<std::string> buf(2);
    int count = 0;

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (int i = 0; i < 10; ++i)
            co_await buf.async_push(std::string("x"));
        buf.close();
    };

    co_await boost::cobalt::join(
        producer(),
        buf.async_drain([&count](const std::string&) { ++count; }));
    co_return count;
}

TEST(SingleBuffer, Backpressure)
{
    EXPECT_EQ(run_sync(do_single_backpressure()), 10);
}

// ===================================================================
// hopping_stream_buffer (cross-executor) x 4 APIs
// ===================================================================

// -- async_drain --

static boost::cobalt::task<std::vector<std::string>>
do_hopping_drain()
{
    detail::hopping_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("a"));
        co_await buf.async_push(std::string("b"));
        co_await buf.async_push(std::string("c"));
        buf.close();
    };

    co_await boost::cobalt::join(
        producer(),
        buf.async_drain([&out](const std::string& s) { out.push_back(s); }));
    co_return out;
}

TEST(HoppingBuffer, Drain)
{
    auto out = run_sync(do_hopping_drain());
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "a");
    EXPECT_EQ(out[1], "b");
    EXPECT_EQ(out[2], "c");
}

// -- async_read --

static boost::cobalt::task<std::vector<std::string>>
do_hopping_read()
{
    detail::hopping_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("x"));
        co_await buf.async_push(std::string("y"));
        buf.close();
    };

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read();
            if (!r) break;
            out.push_back(*r);
        }
    };

    co_await boost::cobalt::join(producer(), consumer());
    co_return out;
}

TEST(HoppingBuffer, Read)
{
    auto out = run_sync(do_hopping_read());
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "x");
    EXPECT_EQ(out[1], "y");
}

// -- async_read_batch --

static boost::cobalt::task<std::vector<std::string>>
do_hopping_batch()
{
    detail::hopping_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (int i = 0; i < 5; ++i)
            co_await buf.async_push(std::to_string(i));
        buf.close();
    };

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read_batch(3);
            if (!r) break;
            for (auto& s : *r) out.push_back(std::move(s));
        }
    };

    co_await boost::cobalt::join(producer(), consumer());
    co_return out;
}

TEST(HoppingBuffer, Batch)
{
    auto out = run_sync(do_hopping_batch());
    ASSERT_EQ(out.size(), 5u);
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(out[i], std::to_string(i));
}

// -- backpressure --

static boost::cobalt::task<int>
do_hopping_backpressure()
{
    detail::hopping_stream_buffer<std::string> buf(2);
    int count = 0;

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (int i = 0; i < 10; ++i)
            co_await buf.async_push(std::string("x"));
        buf.close();
    };

    co_await boost::cobalt::join(
        producer(),
        buf.async_drain([&count](const std::string&) { ++count; }));
    co_return count;
}

TEST(HoppingBuffer, Backpressure)
{
    EXPECT_EQ(run_sync(do_hopping_backpressure()), 10);
}

// ===================================================================
// threadsafe_stream_buffer (SPSC) x 4 APIs
// ===================================================================

// Helper: push N items from a std::thread, then close.
static void threadsafe_push_n(detail::threadsafe_stream_buffer<std::string>& buf,
                              int n, const std::string& prefix)
{
    for (int i = 0; i < n; ++i)
        while (!buf.push(prefix + std::to_string(i))) {}
    buf.close();
}

// -- async_drain --

static boost::cobalt::task<std::vector<std::string>>
do_threadsafe_drain()
{
    detail::threadsafe_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    std::thread producer([&] { threadsafe_push_n(buf, 3, ""); });

    co_await boost::cobalt::join(
        buf.async_forward(),
        buf.async_drain([&out](const std::string& s) { out.push_back(s); }));

    producer.join();
    co_return out;
}

TEST(ThreadsafeBuffer, Drain)
{
    auto out = run_sync(do_threadsafe_drain());
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "0");
    EXPECT_EQ(out[1], "1");
    EXPECT_EQ(out[2], "2");
}

// -- async_read --

static boost::cobalt::task<std::vector<std::string>>
do_threadsafe_read()
{
    detail::threadsafe_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    std::thread producer([&] { threadsafe_push_n(buf, 2, "r"); });

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read();
            if (!r) break;
            out.push_back(*r);
        }
    };

    co_await boost::cobalt::join(buf.async_forward(), consumer());

    producer.join();
    co_return out;
}

TEST(ThreadsafeBuffer, Read)
{
    auto out = run_sync(do_threadsafe_read());
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "r0");
    EXPECT_EQ(out[1], "r1");
}

// -- async_read_batch --

static boost::cobalt::task<std::vector<std::string>>
do_threadsafe_batch()
{
    detail::threadsafe_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    std::thread producer([&] { threadsafe_push_n(buf, 5, "b"); });

    auto consumer = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto r = co_await buf.async_read_batch(3);
            if (!r) break;
            for (auto& s : *r) out.push_back(std::move(s));
        }
    };

    co_await boost::cobalt::join(buf.async_forward(), consumer());

    producer.join();
    co_return out;
}

TEST(ThreadsafeBuffer, Batch)
{
    auto out = run_sync(do_threadsafe_batch());
    ASSERT_EQ(out.size(), 5u);
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(out[i], "b" + std::to_string(i));
}

// -- backpressure (small SPSC queue, retries) --

static boost::cobalt::task<int>
do_threadsafe_backpressure()
{
    detail::threadsafe_stream_buffer<std::string> buf(4);
    int count = 0;

    std::thread producer([&] { threadsafe_push_n(buf, 20, ""); });

    co_await boost::cobalt::join(
        buf.async_forward(),
        buf.async_drain([&count](const std::string&) { ++count; }));

    producer.join();
    co_return count;
}

TEST(ThreadsafeBuffer, Backpressure)
{
    EXPECT_EQ(run_sync(do_threadsafe_backpressure()), 20);
}
