// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for stream_recorder with callback_sink and spdlog_sink.
// file_sink requires BOOST_ASIO_HAS_IO_URING and is not tested here.

#include <binapi2/fapi/streams/detail/sinks/callback_sink.hpp>
#include <binapi2/fapi/streams/detail/sinks/spdlog_sink.hpp>
#include <binapi2/fapi/streams/detail/async_stream_recorder.hpp>
#include <binapi2/fapi/streams/detail/threaded_stream_recorder.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

using namespace binapi2::fapi;

// The recorder uses threadsafe_stream_buffer internally — push() is
// non-blocking and thread-safe, no coroutine context needed on the
// producer side.

// ===================================================================
// callback_sink
// ===================================================================

TEST(ThreadedRecorderCallbackSink, RecordsFrames)
{
    std::vector<std::string> recorded;

    streams::threaded_stream_recorder recorder(16);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));

    recorder.start();

    // threadsafe_stream_buffer::push() is non-blocking and thread-safe.
    {
        while (!buf.push(std::string("frame1"))) {}
        while (!buf.push(std::string("frame2"))) {}
        while (!buf.push(std::string("frame3"))) {}
    }

    recorder.stop();

    ASSERT_EQ(recorded.size(), 3u);
    EXPECT_EQ(recorded[0], "frame1");
    EXPECT_EQ(recorded[1], "frame2");
    EXPECT_EQ(recorded[2], "frame3");
}

TEST(ThreadedRecorderCallbackSink, MultipleStreams)
{
    std::vector<std::string> stream_a;
    std::vector<std::string> stream_b;

    streams::threaded_stream_recorder recorder(64);
    auto& buf_a = recorder.add_stream(
        streams::sinks::callback_sink([&stream_a](const std::string& s) {
            stream_a.push_back(s);
        }));
    auto& buf_b = recorder.add_stream(
        streams::sinks::callback_sink([&stream_b](const std::string& s) {
            stream_b.push_back(s);
        }));

    recorder.start();

    // threadsafe_stream_buffer::push() is non-blocking and thread-safe.
    {
        while (!buf_a.push(std::string("a1"))) {}
        while (!buf_b.push(std::string("b1"))) {}
        while (!buf_a.push(std::string("a2"))) {}
        while (!buf_b.push(std::string("b2"))) {}
    }

    recorder.stop();

    ASSERT_EQ(stream_a.size(), 2u);
    EXPECT_EQ(stream_a[0], "a1");
    EXPECT_EQ(stream_a[1], "a2");

    ASSERT_EQ(stream_b.size(), 2u);
    EXPECT_EQ(stream_b[0], "b1");
    EXPECT_EQ(stream_b[1], "b2");
}

TEST(ThreadedRecorderCallbackSink, EmptyStream)
{
    std::vector<std::string> recorded;

    streams::threaded_stream_recorder recorder(16);
    recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));

    recorder.start();
    recorder.stop();

    EXPECT_TRUE(recorded.empty());
}

TEST(ThreadedRecorderCallbackSink, Backpressure)
{
    int count = 0;

    streams::threaded_stream_recorder recorder(4);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([&count](const std::string&) { ++count; }));

    recorder.start();

    // threadsafe_stream_buffer::push() is non-blocking and thread-safe.
    {
        for (int i = 0; i < 100; ++i)
            while (!buf.push(std::string("x"))) {}
    }

    recorder.stop();
    EXPECT_EQ(count, 100);
}

// ===================================================================
// spdlog_sink
// ===================================================================

TEST(ThreadedRecorderSpdlogSink, RecordsFrames)
{
    std::ostringstream oss;
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    ostream_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::logger>("test_rec", ostream_sink);
    logger->set_level(spdlog::level::info);

    {
        streams::threaded_spdlog_stream_recorder recorder(16);
        auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));

        recorder.start();

        while (!buf.push(std::string(R"({"e":"test1"})"))) {}
        while (!buf.push(std::string(R"({"e":"test2"})"))) {}

        recorder.stop();
    }

    logger->flush();
    spdlog::drop("test_rec");

    std::string output = oss.str();
    EXPECT_NE(output.find(R"({"e":"test1"})"), std::string::npos);
    EXPECT_NE(output.find(R"({"e":"test2"})"), std::string::npos);
}

TEST(ThreadedRecorderSpdlogSink, EmptyStream)
{
    std::ostringstream oss;
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    ostream_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::logger>("test_rec_empty", ostream_sink);

    {
        streams::threaded_spdlog_stream_recorder recorder(16);
        recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        recorder.stop();
    }

    logger->flush();
    spdlog::drop("test_rec_empty");

    EXPECT_TRUE(oss.str().empty());
}

// ===================================================================
// basic_async_stream_recorder — single-executor variant
// ===================================================================

template<typename T>
static T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

static boost::cobalt::task<std::vector<std::string>>
do_async_callback_records()
{
    std::vector<std::string> recorded;

    streams::async_stream_recorder recorder(16);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string("frame1"));
        co_await buf.async_push(std::string("frame2"));
        co_await buf.async_push(std::string("frame3"));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
    co_return recorded;
}

TEST(AsyncRecorderCallbackSink, RecordsFrames)
{
    auto out = run_sync(do_async_callback_records());
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], "frame1");
    EXPECT_EQ(out[1], "frame2");
    EXPECT_EQ(out[2], "frame3");
}

static boost::cobalt::task<std::pair<std::vector<std::string>, std::vector<std::string>>>
do_async_multi_streams()
{
    std::vector<std::string> a, b;

    streams::async_stream_recorder recorder(16);
    auto& buf_a = recorder.add_stream(
        streams::sinks::callback_sink([&a](const std::string& s) { a.push_back(s); }));
    auto& buf_b = recorder.add_stream(
        streams::sinks::callback_sink([&b](const std::string& s) { b.push_back(s); }));

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf_a.async_push(std::string("a1"));
        co_await buf_b.async_push(std::string("b1"));
        co_await buf_a.async_push(std::string("a2"));
        co_await buf_b.async_push(std::string("b2"));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
    co_return std::make_pair(std::move(a), std::move(b));
}

TEST(AsyncRecorderCallbackSink, MultipleStreams)
{
    auto [a, b] = run_sync(do_async_multi_streams());
    ASSERT_EQ(a.size(), 2u);
    EXPECT_EQ(a[0], "a1");
    EXPECT_EQ(a[1], "a2");
    ASSERT_EQ(b.size(), 2u);
    EXPECT_EQ(b[0], "b1");
    EXPECT_EQ(b[1], "b2");
}

static boost::cobalt::task<bool>
do_async_empty()
{
    std::vector<std::string> recorded;
    streams::async_stream_recorder recorder(16);
    recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));
    recorder.close();
    co_await recorder.run();
    co_return recorded.empty();
}

TEST(AsyncRecorderCallbackSink, EmptyStream)
{
    EXPECT_TRUE(run_sync(do_async_empty()));
}

static boost::cobalt::task<int>
do_async_backpressure()
{
    int count = 0;
    streams::async_stream_recorder recorder(4);
    auto& buf = recorder.add_stream(
        streams::sinks::callback_sink([&count](const std::string&) { ++count; }));

    auto producer = [&]() -> boost::cobalt::task<void> {
        for (int i = 0; i < 100; ++i)
            co_await buf.async_push(std::string("x"));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
    co_return count;
}

TEST(AsyncRecorderCallbackSink, Backpressure)
{
    EXPECT_EQ(run_sync(do_async_backpressure()), 100);
}

static boost::cobalt::task<std::string>
do_async_spdlog(std::shared_ptr<spdlog::logger> logger)
{
    streams::async_spdlog_stream_recorder recorder(16);
    auto& buf = recorder.add_stream(streams::sinks::spdlog_sink(logger));

    auto producer = [&]() -> boost::cobalt::task<void> {
        co_await buf.async_push(std::string(R"({"e":"test1"})"));
        co_await buf.async_push(std::string(R"({"e":"test2"})"));
        recorder.close();
    };

    co_await boost::cobalt::join(producer(), recorder.run());
    co_return std::string{};
}

TEST(AsyncRecorderSpdlogSink, RecordsFrames)
{
    std::ostringstream oss;
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    ostream_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::logger>("async_test_rec", ostream_sink);
    logger->set_level(spdlog::level::info);

    run_sync(do_async_spdlog(logger));

    logger->flush();
    spdlog::drop("async_test_rec");

    std::string output = oss.str();
    EXPECT_NE(output.find(R"({"e":"test1"})"), std::string::npos);
    EXPECT_NE(output.find(R"({"e":"test2"})"), std::string::npos);
}
