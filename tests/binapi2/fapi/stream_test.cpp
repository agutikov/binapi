// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for stream components using replay transport.

#include "benchmarks/replay_transport.hpp"

#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/streams/combined_market_stream.hpp>
#include <binapi2/fapi/streams/dynamic_market_stream.hpp>
#include <binapi2/fapi/streams/market_stream.hpp>
#include <binapi2/fapi/streams/user_stream.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <string>
#include <variant>

using namespace binapi2::fapi;
using replay = benchmarks::replay_transport;

// -- Fixtures --

// Note: "e" field must be present for discriminator dispatch even though
// the struct doesn't store it (error_on_unknown_keys = false skips it).
static const std::string book_ticker_json =
    R"({"e":"bookTicker","u":123,"s":"BTCUSDT","b":"71000.00","B":"1.0","a":"71001.00","A":"2.0","T":1000,"E":1001})";

static const std::string depth_json =
    R"({"e":"depthUpdate","E":1001,"T":1000,"s":"BTCUSDT","U":100,"u":200,"pu":99,"b":[["71000.00","1.0"]],"a":[["71001.00","2.0"]]})";

static const std::string account_update_json =
    R"({"e":"ACCOUNT_UPDATE","E":1001,"T":1000,"a":{"m":"ORDER","B":[{"a":"USDT","wb":"10000.00","cw":"9500.00","bc":"50.00"}],"P":[]}})";

static const std::string order_trade_json =
    R"({"e":"ORDER_TRADE_UPDATE","E":1001,"T":1000,"o":{"s":"BTCUSDT","c":"test","S":"BUY","o":"LIMIT","f":"GTC","q":"0.1","p":"71000.00","ap":"71000.00","sp":"0","x":"TRADE","X":"FILLED","i":1,"l":"0.1","z":"0.1","L":"71000.00","N":"USDT","n":"0.07","T":1000,"t":1,"b":"0","a":"0","m":false,"R":false,"wt":"CONTRACT_PRICE","ot":"LIMIT","ps":"LONG","cp":false,"AP":"0","cr":"0","pP":false,"si":0,"ss":0,"rp":"0","V":"NONE","pm":"NONE","gtd":0}})";

static std::string wrap_combined(const std::string& topic, const std::string& event_json)
{
    return R"({"stream":")" + topic + R"(","data":)" + event_json + "}";
}

// Helper: run a cobalt::task synchronously with proper cobalt executor.
template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

// -- Market stream --

static boost::cobalt::task<int>
do_market_subscribe()
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

TEST(MarketStream, SubscribeYieldsEvents)
{
    EXPECT_EQ(run_sync(do_market_subscribe()), 3);
}

static boost::cobalt::task<int>
do_market_no_reconnect()
{
    auto cfg = config::testnet_config();
    streams::basic_market_stream<replay> ms(cfg);
    ms.set_max_reconnects(0);
    ms.transport().messages.assign(1, book_ticker_json);

    int count = 0;
    auto gen = ms.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});
    while (gen) {
        auto ev = co_await gen;
        if (!ev) break;
        ++count;
    }
    co_return count;
}

TEST(MarketStream, StopsAfterExhaust)
{
    EXPECT_EQ(run_sync(do_market_no_reconnect()), 1);
}

// -- User stream --

static boost::cobalt::task<int>
do_user_subscribe()
{
    auto cfg = config::testnet_config();
    streams::basic_user_stream<replay> us(cfg);
    us.set_max_reconnects(0);
    us.transport().messages = { account_update_json, order_trade_json };

    int count = 0;
    auto gen = us.subscribe("fake-key");
    while (gen) {
        auto ev = co_await gen;
        if (!ev) break;
        ++count;
    }
    co_return count;
}

TEST(UserStream, SubscribeYieldsVariantEvents)
{
    EXPECT_EQ(run_sync(do_user_subscribe()), 2);
}

// -- Combined market stream --

static boost::cobalt::task<int>
do_combined_subscribe()
{
    auto cfg = config::testnet_config();
    streams::basic_combined_market_stream<replay> cs(cfg);
    cs.set_max_reconnects(0);
    cs.transport().messages = {
        wrap_combined("btcusdt@bookTicker", book_ticker_json),
        wrap_combined("btcusdt@depth@100ms", depth_json),
        wrap_combined("btcusdt@bookTicker", book_ticker_json),
    };

    using V = std::variant<types::book_ticker_stream_event_t, types::depth_stream_event_t>;
    auto gen = cs.subscribe<V>(
        types::book_ticker_subscription{.symbol = "BTCUSDT"},
        types::diff_book_depth_subscription{.symbol = "BTCUSDT", .speed = "100ms"});

    int count = 0;
    std::string last_err;
    while (gen) {
        auto ev = co_await gen;
        if (!ev) { last_err = ev.err.message; break; }
        ++count;
    }
    if (count == 0) std::fprintf(stderr, "combined error: %s\n", last_err.c_str());
    co_return count;
}

TEST(CombinedMarketStream, SubscribeYieldsVariantEvents)
{
    EXPECT_EQ(run_sync(do_combined_subscribe()), 3);
}

// -- Dynamic market stream --

static boost::cobalt::task<int>
do_dynamic_read()
{
    auto cfg = config::testnet_config();
    streams::basic_dynamic_market_stream<replay> ds(cfg);
    ds.transport().messages = {
        wrap_combined("btcusdt@bookTicker", book_ticker_json),
        wrap_combined("btcusdt@depth@100ms", depth_json),
    };

    co_await ds.async_connect();

    int count = 0;
    for (int i = 0; i < 2; ++i) {
        auto msg = co_await ds.async_read_event();
        if (!msg) break;
        if (std::holds_alternative<types::market_stream_event_t>(*msg))
            ++count;
    }
    co_return count;
}

TEST(DynamicMarketStream, ReadEventParsesData)
{
    EXPECT_EQ(run_sync(do_dynamic_read()), 2);
}

static boost::cobalt::task<int>
do_dynamic_control()
{
    auto cfg = config::testnet_config();
    streams::basic_dynamic_market_stream<replay> ds(cfg);
    ds.transport().messages = {
        R"({"result":null,"id":1})",
        wrap_combined("btcusdt@bookTicker", book_ticker_json),
    };

    co_await ds.async_connect();

    int controls = 0;
    int events = 0;
    for (int i = 0; i < 2; ++i) {
        auto msg = co_await ds.async_read_event();
        if (!msg) break;
        if (std::holds_alternative<streams::control_response_t>(*msg)) ++controls;
        if (std::holds_alternative<types::market_stream_event_t>(*msg)) ++events;
    }
    co_return controls * 10 + events;
}

TEST(DynamicMarketStream, ControlResponseSeparateFromData)
{
    EXPECT_EQ(run_sync(do_dynamic_control()), 11); // 1 control, 1 event
}

// -- parse_user_event --

TEST(ParseUserEvent, AccountUpdate)
{
    auto ev = streams::parse_user_event(account_update_json);
    EXPECT_TRUE(ev);
    EXPECT_TRUE(std::holds_alternative<types::account_update_event_t>(*ev));
}

TEST(ParseUserEvent, OrderTradeUpdate)
{
    auto ev = streams::parse_user_event(order_trade_json);
    EXPECT_TRUE(ev);
    EXPECT_TRUE(std::holds_alternative<types::order_trade_update_event_t>(*ev));
}

TEST(ParseUserEvent, UnknownType)
{
    auto ev = streams::parse_user_event(R"({"e":"UNKNOWN","E":1})");
    EXPECT_FALSE(ev);
}

// -- Combined frame parsing --

TEST(CombinedFrame, ParseWrapper)
{
    std::string wrapped = wrap_combined("btcusdt@bookTicker", book_ticker_json);
    types::combined_stream_frame_t f{};
    glz::context ctx{};
    auto ec = glz::read<detail::json_read_opts>(f, wrapped, ctx);
    ASSERT_FALSE(ec) << glz::format_error(ec, wrapped);
    EXPECT_EQ(f.stream, "btcusdt@bookTicker");
    EXPECT_FALSE(f.data.str.empty());
}

TEST(CombinedFrame, ParseEventFromData)
{
    std::string wrapped = wrap_combined("btcusdt@bookTicker", book_ticker_json);
    types::combined_stream_frame_t f{};
    glz::context ctx{};
    ASSERT_FALSE(glz::read<detail::json_read_opts>(f, wrapped, ctx));

    types::book_ticker_stream_event_t ev{};
    glz::context ctx2{};
    auto ec = glz::read<detail::json_read_opts>(ev, f.data.str, ctx2);
    ASSERT_FALSE(ec) << glz::format_error(ec, f.data.str);
    EXPECT_EQ(std::string(ev.symbol), "BTCUSDT");
}

// -- variant_entry discriminator lifetime --

TEST(VariantEntry, EnumBasedDispatch)
{
    using V = std::variant<types::book_ticker_stream_event_t, types::depth_stream_event_t>;
    using E = types::market_event_type_t;
    constexpr detail::variant_entry<E, V> dispatch[] = {
        detail::make_entry<types::book_ticker_stream_event_t, E, V>(types::event_traits<types::book_ticker_stream_event_t>::enum_value),
        detail::make_entry<types::depth_stream_event_t, E, V>(types::event_traits<types::depth_stream_event_t>::enum_value),
    };
    auto parsed = detail::dispatch_variant<E, V>(E::book_ticker, book_ticker_json, dispatch);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(std::holds_alternative<types::book_ticker_stream_event_t>(*parsed));
}

TEST(VariantEntry, EnumDispatchUnknown)
{
    using V = std::variant<types::book_ticker_stream_event_t>;
    using E = types::market_event_type_t;
    constexpr detail::variant_entry<E, V> dispatch[] = {
        detail::make_entry<types::book_ticker_stream_event_t, E, V>(types::event_traits<types::book_ticker_stream_event_t>::enum_value),
    };
    // depth_update not in dispatch table
    auto parsed = detail::dispatch_variant<E, V>(E::depth_update, depth_json, dispatch);
    EXPECT_FALSE(parsed.has_value());
}

TEST(VariantEntry, ParseVariantByEventField)
{
    using V = std::variant<types::book_ticker_stream_event_t, types::depth_stream_event_t>;
    using E = types::market_event_type_t;
    constexpr detail::variant_entry<E, V> dispatch[] = {
        detail::make_entry<types::book_ticker_stream_event_t, E, V>(types::event_traits<types::book_ticker_stream_event_t>::enum_value),
        detail::make_entry<types::depth_stream_event_t, E, V>(types::event_traits<types::depth_stream_event_t>::enum_value),
    };
    // parse_variant extracts "e" from JSON, converts to enum, dispatches
    auto parsed = detail::parse_variant<E, V>("e", book_ticker_json, dispatch);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(std::holds_alternative<types::book_ticker_stream_event_t>(*parsed));
}

// -- stream_traits topic vs target --

TEST(StreamTraits, TopicDoesNotIncludeBasePath)
{
    auto cfg = config::testnet_config();
    auto topic = streams::stream_traits<types::book_ticker_subscription>::topic(
        cfg, types::book_ticker_subscription{.symbol = "BTCUSDT"});
    EXPECT_EQ(topic, "btcusdt@bookTicker");
    EXPECT_TRUE(topic.find("/ws") == std::string::npos);
}

TEST(StreamTraits, TargetIncludesBasePath)
{
    auto cfg = config::testnet_config();
    auto target = streams::stream_traits<types::book_ticker_subscription>::target(
        cfg, types::book_ticker_subscription{.symbol = "BTCUSDT"});
    EXPECT_EQ(target, "/ws/btcusdt@bookTicker");
}

// -- Dynamic frame parsing --

TEST(DynamicFrame, ParseDataFrame)
{
    std::string wrapped = wrap_combined("btcusdt@bookTicker", book_ticker_json);
    types::combined_stream_frame_t f{};
    glz::context ctx{};
    auto ec = glz::read<detail::json_read_opts>(f, wrapped, ctx);
    ASSERT_FALSE(ec) << glz::format_error(ec, wrapped);
    EXPECT_EQ(f.stream, "btcusdt@bookTicker");
}

TEST(DynamicFrame, ParseControlResponse)
{
    std::string json = R"({"result":null,"id":1})";
    streams::detail::dynamic_control_response cr{};
    glz::context ctx{};
    auto ec = glz::read<detail::json_read_opts>(cr, json, ctx);
    ASSERT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_EQ(cr.id, 1u);
}

TEST(DynamicFrame, ParseListResponse)
{
    std::string json = R"({"result":["btcusdt@bookTicker","btcusdt@depth@100ms"],"id":3})";
    streams::detail::dynamic_control_response cr{};
    glz::context ctx{};
    auto ec = glz::read<detail::json_read_opts>(cr, json, ctx);
    ASSERT_FALSE(ec) << glz::format_error(ec, json);
    ASSERT_TRUE(cr.result.has_value());
    EXPECT_EQ(cr.result->size(), 2u);
    EXPECT_EQ(cr.id, 3u);
}

// -- market_event_t variant dispatch by "e" field --

TEST(MarketEventDispatch, BookTickerByEventType)
{
    using E = types::market_event_type_t;
    using V = types::market_stream_event_t;
    auto parsed = detail::parse_variant<E, V>(
        "e", book_ticker_json, streams::detail::market_event_mapping);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(std::holds_alternative<types::book_ticker_stream_event_t>(*parsed));
}

TEST(MarketEventDispatch, DepthByEventType)
{
    using E = types::market_event_type_t;
    using V = types::market_stream_event_t;
    auto parsed = detail::parse_variant<E, V>(
        "e", depth_json, streams::detail::market_event_mapping);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(std::holds_alternative<types::depth_stream_event_t>(*parsed));
}

// -- extract_string_field --

TEST(ExtractStringField, SimpleKey)
{
    std::string json = R"({"e":"bookTicker","s":"BTCUSDT"})";
    auto val = detail::extract_string_field(json, "e");
    EXPECT_EQ(val, "bookTicker");
}

TEST(ExtractStringField, NestedKeyIgnored)
{
    // "e" inside nested object should NOT be found (only top-level)
    std::string json = R"({"data":{"e":"bookTicker"},"s":"BTCUSDT"})";
    auto val = detail::extract_string_field(json, "e");
    EXPECT_TRUE(val.empty());
}

TEST(ExtractStringField, MissingKey)
{
    std::string json = R"({"s":"BTCUSDT"})";
    auto val = detail::extract_string_field(json, "e");
    EXPECT_TRUE(val.empty());
}

// -- Stream buffer recording --

static boost::cobalt::task<std::vector<std::string>>
do_buffer_with_connection()
{
    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay> conn(cfg);
    conn.transport().messages = {book_ticker_json, depth_json, book_ticker_json};

    std::vector<std::string> recorded;
    detail::stream_buffer<std::string> buffer(16);
    conn.attach_buffer(buffer);

    co_await conn.async_connect("", "", "/ws/test");

    auto read_all = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto msg = co_await conn.async_read_text();
            if (!msg) break;
        }
        buffer.close();
    };

    co_await boost::cobalt::join(
        read_all(),
        buffer.async_drain([&recorded](const std::string& s) { recorded.push_back(s); }));
    co_return recorded;
}

TEST(StreamBuffer, ConnectionRecordsAllFrames)
{
    auto recorded = run_sync(do_buffer_with_connection());
    ASSERT_EQ(recorded.size(), 3u);
    EXPECT_EQ(recorded[0], book_ticker_json);
    EXPECT_EQ(recorded[1], depth_json);
    EXPECT_EQ(recorded[2], book_ticker_json);
}

static boost::cobalt::task<std::vector<std::string>>
do_buffer_with_market_stream()
{
    auto cfg = config::testnet_config();
    streams::basic_market_stream<replay> ms(cfg);
    ms.set_max_reconnects(0);
    ms.transport().messages.assign(5, book_ticker_json);

    std::vector<std::string> recorded;
    detail::stream_buffer<std::string> buffer(16);
    ms.connection().attach_buffer(buffer);

    auto consume = [&]() -> boost::cobalt::task<void> {
        auto gen = ms.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});
        while (gen) {
            auto ev = co_await gen;
            if (!ev) break;
        }
        buffer.close();
    };

    co_await boost::cobalt::join(
        consume(),
        buffer.async_drain([&recorded](const std::string& s) { recorded.push_back(s); }));
    co_return recorded;
}

TEST(StreamBuffer, MarketStreamRecordsAllFrames)
{
    auto recorded = run_sync(do_buffer_with_market_stream());
    EXPECT_EQ(recorded.size(), 5u);
    for (const auto& frame : recorded) {
        EXPECT_EQ(frame, book_ticker_json);
    }
}

static boost::cobalt::task<std::vector<std::string>>
do_buffer_with_user_stream()
{
    auto cfg = config::testnet_config();
    streams::basic_user_stream<replay> us(cfg);
    us.set_max_reconnects(0);
    us.transport().messages = {account_update_json, order_trade_json};

    std::vector<std::string> recorded;
    detail::stream_buffer<std::string> buffer(16);
    us.connection().attach_buffer(buffer);

    auto consume = [&]() -> boost::cobalt::task<void> {
        auto gen = us.subscribe("fake-key");
        while (gen) {
            auto ev = co_await gen;
            if (!ev) break;
        }
        buffer.close();
    };

    co_await boost::cobalt::join(
        consume(),
        buffer.async_drain([&recorded](const std::string& s) { recorded.push_back(s); }));
    co_return recorded;
}

TEST(StreamBuffer, UserStreamRecordsAllFrames)
{
    auto recorded = run_sync(do_buffer_with_user_stream());
    ASSERT_EQ(recorded.size(), 2u);
    EXPECT_EQ(recorded[0], account_update_json);
    EXPECT_EQ(recorded[1], order_trade_json);
}

static boost::cobalt::task<std::vector<std::string>>
do_buffer_backpressure()
{
    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay> conn(cfg);
    conn.transport().messages.assign(10, book_ticker_json);

    std::vector<std::string> recorded;
    detail::stream_buffer<std::string> buffer(2);
    conn.attach_buffer(buffer);

    co_await conn.async_connect("", "", "/ws/test");

    auto read_all = [&]() -> boost::cobalt::task<void> {
        while (true) {
            auto msg = co_await conn.async_read_text();
            if (!msg) break;
        }
        buffer.close();
    };

    co_await boost::cobalt::join(
        read_all(),
        buffer.async_drain([&recorded](const std::string& s) { recorded.push_back(s); }));
    co_return recorded;
}

TEST(StreamBuffer, BackpressurePreservesAllFrames)
{
    auto recorded = run_sync(do_buffer_backpressure());
    EXPECT_EQ(recorded.size(), 10u);
}

static boost::cobalt::task<std::vector<std::string>>
do_buffer_no_frames()
{
    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay> conn(cfg);
    conn.transport().messages = {};

    std::vector<std::string> recorded;
    detail::stream_buffer<std::string> buffer(16);
    conn.attach_buffer(buffer);

    co_await conn.async_connect("", "", "/ws/test");

    auto read_all = [&]() -> boost::cobalt::task<void> {
        [[maybe_unused]] auto msg = co_await conn.async_read_text();
        buffer.close();
    };

    co_await boost::cobalt::join(
        read_all(),
        buffer.async_drain([&recorded](const std::string& s) { recorded.push_back(s); }));
    co_return recorded;
}

TEST(StreamBuffer, NoFramesProducesEmptyRecording)
{
    auto recorded = run_sync(do_buffer_no_frames());
    EXPECT_TRUE(recorded.empty());
}

static boost::cobalt::task<int>
do_no_buffer_on_connection()
{
    auto cfg = config::testnet_config();
    streams::basic_stream_connection<replay> conn(cfg);
    conn.transport().messages = {book_ticker_json, depth_json};

    co_await conn.async_connect("", "", "/ws/test");

    int count = 0;
    while (true) {
        auto msg = co_await conn.async_read_text();
        if (!msg) break;
        ++count;
    }
    co_return count;
}

TEST(StreamBuffer, ConnectionWithoutBufferStillWorks)
{
    EXPECT_EQ(run_sync(do_no_buffer_on_connection()), 2);
}
