// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for stream_buffer variants and stream_recorder.

#include <binapi2/fapi/detail/hopping_stream_buffer.hpp>
#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>
#include <binapi2/fapi/streams/sinks/callback_sink.hpp>
#include <binapi2/fapi/streams/stream_recorder.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace binapi2::fapi;

template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

// -- stream_buffer (single-executor) --

static boost::cobalt::task<std::vector<std::string>>
do_buffer_push_drain()
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

TEST(StreamBufferSingle, PushDrain)
{
    auto out = run_sync(do_buffer_push_drain());
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "a");
    EXPECT_EQ(out[1], "b");
    EXPECT_EQ(out[2], "c");
}

static boost::cobalt::task<std::vector<std::string>>
do_buffer_push_read()
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

TEST(StreamBufferSingle, PushRead)
{
    auto out = run_sync(do_buffer_push_read());
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "x");
    EXPECT_EQ(out[1], "y");
}

static boost::cobalt::task<std::vector<std::string>>
do_buffer_push_batch()
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

TEST(StreamBufferSingle, PushBatch)
{
    auto out = run_sync(do_buffer_push_batch());
    ASSERT_EQ(out.size(), 5u);
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(out[i], std::to_string(i));
}

static boost::cobalt::task<int>
do_buffer_backpressure()
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

TEST(StreamBufferSingle, Backpressure)
{
    EXPECT_EQ(run_sync(do_buffer_backpressure()), 10);
}

static boost::cobalt::task<int>
do_buffer_empty()
{
    detail::stream_buffer<std::string> buf(16);
    buf.close();
    auto r = co_await buf.async_read();
    co_return r ? 1 : 0;
}

TEST(StreamBufferSingle, EmptyClose)
{
    EXPECT_EQ(run_sync(do_buffer_empty()), 0);
}

// -- hopping_stream_buffer (cross-executor, same-executor test) --

static boost::cobalt::task<std::vector<std::string>>
do_hopping_same_executor()
{
    detail::hopping_stream_buffer<std::string> buf(16);
    std::vector<std::string> out;

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("h1"));
        co_await buf.async_push(std::string("h2"));
        buf.close();
    };

    co_await boost::cobalt::join(
        producer(),
        buf.async_drain([&out](const std::string& s) { out.push_back(s); }));
    co_return out;
}

TEST(HoppingStreamBuffer, SameExecutorWorks)
{
    auto out = run_sync(do_hopping_same_executor());
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "h1");
    EXPECT_EQ(out[1], "h2");
}

// -- stream_recorder (cross-executor with io_thread) --

TEST(StreamRecorder, CallbackSinkRecordsFrames)
{
    std::vector<std::string> recorded;

    streams::stream_recorder recorder(16);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));

    recorder.start();

    // Push frames from this thread (uses hopping_stream_buffer cross-executor)
    boost::cobalt::run([&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("frame1"));
        co_await buf.async_push(std::string("frame2"));
        co_await buf.async_push(std::string("frame3"));
        buf.close();
    }());

    recorder.stop();

    // Give the drain coroutine a moment to finish processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(recorded.size(), 3u);
    EXPECT_EQ(recorded[0], "frame1");
    EXPECT_EQ(recorded[1], "frame2");
    EXPECT_EQ(recorded[2], "frame3");
}

TEST(StreamRecorder, MultipleStreams)
{
    std::vector<std::string> stream_a;
    std::vector<std::string> stream_b;

    streams::stream_recorder recorder(16);
    auto& buf_a = recorder.add_stream(
        streams::sinks::callback_sink([&stream_a](const std::string& s) {
            stream_a.push_back(s);
        }));
    auto& buf_b = recorder.add_stream(
        streams::sinks::callback_sink([&stream_b](const std::string& s) {
            stream_b.push_back(s);
        }));

    recorder.start();

    boost::cobalt::run([&]() -> boost::cobalt::task<void> {
        co_await buf_a.async_push(std::string("a1"));
        co_await buf_b.async_push(std::string("b1"));
        co_await buf_a.async_push(std::string("a2"));
        co_await buf_b.async_push(std::string("b2"));
        buf_a.close();
        buf_b.close();
    }());

    recorder.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(stream_a.size(), 2u);
    EXPECT_EQ(stream_a[0], "a1");
    EXPECT_EQ(stream_a[1], "a2");

    ASSERT_EQ(stream_b.size(), 2u);
    EXPECT_EQ(stream_b[0], "b1");
    EXPECT_EQ(stream_b[1], "b2");
}

TEST(StreamRecorder, EmptyStream)
{
    std::vector<std::string> recorded;

    streams::stream_recorder recorder(16);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));

    recorder.start();
    buf.close();
    recorder.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(recorded.empty());
}
