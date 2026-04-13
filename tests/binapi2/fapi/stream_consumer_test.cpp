// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for stream_consumer concept, built-in consumers,
// and the stream_parser utility.

#include "benchmarks/replay_transport.hpp"

#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>
#include <binapi2/fapi/streams/market_stream.hpp>
#include <binapi2/fapi/streams/detail/stream_consumer.hpp>
#include <binapi2/fapi/streams/detail/stream_parser.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <string>
#include <thread>
#include <vector>

using namespace binapi2::fapi;
using replay = benchmarks::replay_transport;

template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

static const std::string book_ticker_json =
    R"({"e":"bookTicker","u":123,"s":"BTCUSDT","b":"71000.00","B":"1.0","a":"71001.00","A":"2.0","T":1000,"E":1001})";

// -- Concept checks --

TEST(StreamConsumer, InlineConsumerSatisfiesConcept)
{
    static_assert(streams::stream_consumer<streams::inline_consumer>);
}

TEST(StreamConsumer, BufferConsumerSatisfiesConcept)
{
    static_assert(streams::stream_consumer<
                  streams::buffer_consumer<detail::stream_buffer<std::string>>>);
}

TEST(StreamConsumer, ThreadsafeBufferConsumerSatisfiesConcept)
{
    static_assert(streams::stream_consumer<
                  streams::buffer_consumer<detail::threadsafe_stream_buffer<std::string>>>);
}

// -- inline_consumer: zero overhead, existing behavior preserved --

static boost::cobalt::task<int>
do_inline_consumer()
{
    auto cfg = config::testnet_config();
    streams::basic_market_stream<replay> ms(cfg);
    ms.set_max_reconnects(0);
    ms.transport().messages.assign(3, book_ticker_json);

    int count = 0;
    auto gen = ms.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});
    while (gen) {
        auto ev = co_await gen;
        if (!ev) break;
        ++count;
    }
    co_return count;
}

TEST(StreamConsumer, InlineConsumerPreservesExistingBehavior)
{
    EXPECT_EQ(run_sync(do_inline_consumer()), 3);
}

// -- buffer_consumer with stream_buffer (same-executor) --

static boost::cobalt::task<std::vector<std::string>>
do_buffer_consumer_same_executor()
{
    detail::stream_buffer<std::string> buf(16);
    using consumer_type = streams::buffer_consumer<detail::stream_buffer<std::string>>;

    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay, consumer_type> conn(cfg, consumer_type(buf));
    conn.transport().messages = {book_ticker_json, book_ticker_json};

    co_await conn.async_connect("", "", "/ws/test");

    std::vector<std::string> captured;

    auto read_all = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto msg = co_await conn.async_read_text();
            if (!msg) break;
        }
        buf.close();
    };

    co_await boost::cobalt::join(
        read_all(),
        buf.async_drain([&captured](const std::string& s) { captured.push_back(s); }));

    co_return captured;
}

TEST(StreamConsumer, BufferConsumerCapturesFrames)
{
    auto captured = run_sync(do_buffer_consumer_same_executor());
    ASSERT_EQ(captured.size(), 2u);
    EXPECT_EQ(captured[0], book_ticker_json);
    EXPECT_EQ(captured[1], book_ticker_json);
}

// -- buffer_consumer with threadsafe_stream_buffer (cross-thread) --

static boost::cobalt::task<std::vector<std::string>>
do_threadsafe_consumer()
{
    detail::threadsafe_stream_buffer<std::string> buf(16);
    using consumer_type = streams::buffer_consumer<detail::threadsafe_stream_buffer<std::string>>;

    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay, consumer_type> conn(cfg, consumer_type(buf));
    conn.transport().messages.assign(5, book_ticker_json);

    co_await conn.async_connect("", "", "/ws/test");

    std::vector<std::string> captured;

    auto read_all = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto msg = co_await conn.async_read_text();
            if (!msg) break;
        }
        conn.consumer().close();
    };

    co_await boost::cobalt::join(
        read_all(),
        buf.async_forward(),
        buf.async_drain([&captured](const std::string& s) { captured.push_back(s); }));

    co_return captured;
}

TEST(StreamConsumer, ThreadsafeConsumerCapturesFrames)
{
    auto captured = run_sync(do_threadsafe_consumer());
    EXPECT_EQ(captured.size(), 5u);
}

// -- fan_out_consumer: multiple consumers --

static boost::cobalt::task<int>
do_fan_out_consumer()
{
    detail::stream_buffer<std::string> buf_a(16);
    detail::stream_buffer<std::string> buf_b(16);

    using consumer_a = streams::buffer_consumer<detail::stream_buffer<std::string>>;
    using consumer_b = streams::buffer_consumer<detail::stream_buffer<std::string>>;
    using fan_out = streams::fan_out_consumer<consumer_a, consumer_b>;

    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay, fan_out> conn(
        cfg, fan_out(consumer_a(buf_a), consumer_b(buf_b)));
    conn.transport().messages.assign(3, book_ticker_json);

    co_await conn.async_connect("", "", "/ws/test");

    int count_a = 0;
    int count_b = 0;

    auto read_all = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto msg = co_await conn.async_read_text();
            if (!msg) break;
        }
        buf_a.close();
        buf_b.close();
    };

    co_await boost::cobalt::join(
        read_all(),
        buf_a.async_drain([&count_a](const std::string&) { ++count_a; }),
        buf_b.async_drain([&count_b](const std::string&) { ++count_b; }));

    co_return count_a * 10 + count_b;
}

TEST(StreamConsumer, FanOutConsumerFeedsMultipleBuffers)
{
    EXPECT_EQ(run_sync(do_fan_out_consumer()), 33); // 3 each
}

// -- stream_parser: string → typed event --

static boost::cobalt::task<int>
do_stream_parser()
{
    detail::threadsafe_stream_buffer<std::string> raw_buf(16);
    detail::threadsafe_stream_buffer<types::book_ticker_stream_event_t> event_buf(16);

    auto parse_fn = [](const std::string& json)
        -> result<types::book_ticker_stream_event_t> {
        types::book_ticker_stream_event_t event{};
        glz::context ctx{};
        auto ec = glz::read<detail::json_read_opts>(event, json, ctx);
        if (ec)
            return result<types::book_ticker_stream_event_t>::failure(
                {error_code::json, 0, 0, "parse error", json});
        return result<types::book_ticker_stream_event_t>::success(std::move(event));
    };

    streams::stream_parser<types::book_ticker_stream_event_t> parser(
        raw_buf, event_buf, parse_fn);

    // Push raw frames from a separate thread
    std::thread producer([&] {
        for (int i = 0; i < 3; ++i)
            while (!raw_buf.push(std::string(book_ticker_json))) {}
        raw_buf.close();
    });

    int count = 0;

    auto do_all = [&]() -> boost::cobalt::task<void> {
        auto parser_done = [&]() -> boost::cobalt::task<void> {
            co_await parser.async_run();
        };

        auto consumer_done = [&]() -> boost::cobalt::task<void> {
            co_await boost::cobalt::join(
                event_buf.async_forward(),
                [&]() -> boost::cobalt::task<void> {
                    while (true) {
                        auto ev = co_await event_buf.async_read();
                        if (!ev) break;
                        ++count;
                    }
                }());
        };

        co_await boost::cobalt::join(parser_done(), consumer_done());
    };

    co_await do_all();
    producer.join();
    co_return count;
}

TEST(StreamParser, ParsesRawFramesIntoTypedEvents)
{
    EXPECT_EQ(run_sync(do_stream_parser()), 3);
}
