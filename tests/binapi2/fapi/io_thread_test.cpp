// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for io_thread — verifies that cobalt generators work
// on the background thread (not just tasks).

#include "benchmarks/replay_transport.hpp"

#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/streams/market_stream.hpp>
#include <binapi2/fapi/streams/user_stream.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <string>

using namespace binapi2::fapi;
using replay = benchmarks::replay_transport;

static const std::string book_ticker_json =
    R"({"e":"bookTicker","u":123,"s":"BTCUSDT","b":"71000.00","B":"1.0","a":"71001.00","A":"2.0","T":1000,"E":1001})";

static const std::string account_update_json =
    R"({"e":"ACCOUNT_UPDATE","E":1001,"T":1000,"a":{"m":"ORDER","B":[{"a":"USDT","wb":"10000.00","cw":"9500.00","bc":"50.00"}],"P":[]}})";

// -- Basic generator on io_thread --

static boost::cobalt::generator<int> simple_generator(int n)
{
    for (int i = 1; i <= n; ++i)
        co_yield i;
    co_yield -1; // sentinel — cobalt generators must not fall off the end
}

static boost::cobalt::task<int> consume_simple_generator()
{
    auto gen = simple_generator(3);
    int sum = 0;
    while (gen) {
        int v = co_await gen;
        if (v < 0) break;
        sum += v;
    }
    co_return sum;
}

TEST(IoThread, SimpleGeneratorWorks)
{
    detail::io_thread io;
    EXPECT_EQ(io.run_sync(consume_simple_generator()), 6);
}

// -- Market stream generator on io_thread --

static boost::cobalt::task<int> market_stream_on_io_thread()
{
    auto cfg = config::testnet_config();
    streams::basic_market_stream<replay> ms(cfg);
    ms.set_max_reconnects(0);
    ms.transport().messages.assign(5, book_ticker_json);

    int count = 0;
    auto gen = ms.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});
    while (gen) {
        auto ev = co_await gen;
        if (!ev) break;
        ++count;
    }
    co_return count;
}

TEST(IoThread, MarketStreamGeneratorWorks)
{
    detail::io_thread io;
    EXPECT_EQ(io.run_sync(market_stream_on_io_thread()), 5);
}

// -- User stream variant generator on io_thread --

static boost::cobalt::task<int> user_stream_on_io_thread()
{
    auto cfg = config::testnet_config();
    streams::basic_user_stream<replay> us(cfg);
    us.set_max_reconnects(0);
    us.transport().messages = { account_update_json };

    int count = 0;
    auto gen = us.subscribe("fake-key");
    while (gen) {
        auto ev = co_await gen;
        if (!ev) break;
        ++count;
    }
    co_return count;
}

TEST(IoThread, UserStreamGeneratorWorks)
{
    detail::io_thread io;
    EXPECT_EQ(io.run_sync(user_stream_on_io_thread()), 1);
}

// -- Multiple run_sync calls on same io_thread --

TEST(IoThread, MultipleRunSyncCalls)
{
    detail::io_thread io;
    EXPECT_EQ(io.run_sync(consume_simple_generator()), 6);
    EXPECT_EQ(io.run_sync(consume_simple_generator()), 6);
    EXPECT_EQ(io.run_sync(market_stream_on_io_thread()), 5);
}
