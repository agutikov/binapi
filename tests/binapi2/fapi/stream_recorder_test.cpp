// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for stream_recorder with callback_sink and spdlog_sink.
// file_sink requires BOOST_ASIO_HAS_IO_URING and is not tested here.

#include <binapi2/fapi/streams/sinks/callback_sink.hpp>
#include <binapi2/fapi/streams/sinks/spdlog_sink.hpp>
#include <binapi2/fapi/streams/stream_recorder.hpp>

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

TEST(RecorderCallbackSink, RecordsFrames)
{
    std::vector<std::string> recorded;

    streams::stream_recorder recorder(16);
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

TEST(RecorderCallbackSink, MultipleStreams)
{
    std::vector<std::string> stream_a;
    std::vector<std::string> stream_b;

    streams::stream_recorder recorder(64);
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

TEST(RecorderCallbackSink, EmptyStream)
{
    std::vector<std::string> recorded;

    streams::stream_recorder recorder(16);
    recorder.add_stream(
        streams::sinks::callback_sink([&recorded](const std::string& s) {
            recorded.push_back(s);
        }));

    recorder.start();
    recorder.stop();

    EXPECT_TRUE(recorded.empty());
}

TEST(RecorderCallbackSink, Backpressure)
{
    int count = 0;

    streams::stream_recorder recorder(4);
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

TEST(RecorderSpdlogSink, RecordsFrames)
{
    std::ostringstream oss;
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    ostream_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::logger>("test_rec", ostream_sink);
    logger->set_level(spdlog::level::info);

    {
        streams::spdlog_stream_recorder recorder(16);
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

TEST(RecorderSpdlogSink, EmptyStream)
{
    std::ostringstream oss;
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    ostream_sink->set_pattern("%v");
    auto logger = std::make_shared<spdlog::logger>("test_rec_empty", ostream_sink);

    {
        streams::spdlog_stream_recorder recorder(16);
        recorder.add_stream(streams::sinks::spdlog_sink(logger));
        recorder.start();
        recorder.stop();
    }

    logger->flush();
    spdlog::drop("test_rec_empty");

    EXPECT_TRUE(oss.str().empty());
}
