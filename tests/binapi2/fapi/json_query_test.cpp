// SPDX-License-Identifier: Apache-2.0
//
// Tests for JSON serialization round-trips and to_query_map conversion.

#include <binapi2/fapi/query.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <gtest/gtest.h>
#include <glaze/glaze.hpp>

#include <string>

namespace {

using namespace binapi2::fapi::types;

// ============================================================================
// Helper: parse JSON into T, asserting success.
// ============================================================================

template<class T>
T parse_json(std::string_view json)
{
    T obj{};
    auto ec = glz::read_json(obj, json);
    EXPECT_FALSE(ec) << "parse error: " << glz::format_error(ec, json);
    return obj;
}

template<class T>
std::string to_json(const T& obj)
{
    std::string buf;
    (void)glz::write_json(obj, buf);
    return buf;
}

// ============================================================================
// Part 1: JSON round-trips for types in common.hpp
// ============================================================================

// ---------- binance_error_document ----------

TEST(JsonRoundTrip, BinanceErrorDocument)
{
    constexpr std::string_view input = R"({"code":-1021,"msg":"Timestamp outside"})";
    auto doc = parse_json<binance_error_document>(input);

    EXPECT_EQ(doc.code, -1021);
    EXPECT_EQ(doc.msg, "Timestamp outside");

    // Re-serialize and parse again.
    auto json2 = to_json(doc);
    auto doc2 = parse_json<binance_error_document>(json2);
    EXPECT_EQ(doc2.code, doc.code);
    EXPECT_EQ(doc2.msg, doc.msg);
}

// ---------- server_time_response ----------

TEST(JsonRoundTrip, ServerTimeResponse)
{
    constexpr std::string_view input = R"({"serverTime":1234567890123})";
    auto resp = parse_json<server_time_response>(input);

    EXPECT_EQ(resp.serverTime, timestamp_ms_t{1234567890123ULL});

    auto json2 = to_json(resp);
    auto resp2 = parse_json<server_time_response>(json2);
    EXPECT_EQ(resp2.serverTime, resp.serverTime);
}

// ---------- rate_limit ----------

TEST(JsonRoundTrip, RateLimitWithoutCount)
{
    constexpr std::string_view input =
        R"({"rateLimitType":"REQUEST_WEIGHT","interval":"MINUTE","intervalNum":1,"limit":2400})";
    auto rl = parse_json<rate_limit>(input);

    EXPECT_EQ(rl.rateLimitType, rate_limit_type::request_weight);
    EXPECT_EQ(rl.interval, rate_limit_interval::minute);
    EXPECT_EQ(rl.intervalNum, 1);
    EXPECT_EQ(rl.limit, 2400);
    EXPECT_FALSE(rl.count.has_value());

    auto json2 = to_json(rl);
    auto rl2 = parse_json<rate_limit>(json2);
    EXPECT_EQ(rl2.rateLimitType, rl.rateLimitType);
    EXPECT_EQ(rl2.interval, rl.interval);
    EXPECT_EQ(rl2.intervalNum, rl.intervalNum);
    EXPECT_EQ(rl2.limit, rl.limit);
    EXPECT_FALSE(rl2.count.has_value());
}

TEST(JsonRoundTrip, RateLimitWithCount)
{
    constexpr std::string_view input =
        R"({"rateLimitType":"ORDERS_1M","interval":"MINUTE","intervalNum":1,"limit":1200,"count":42})";
    auto rl = parse_json<rate_limit>(input);

    EXPECT_EQ(rl.rateLimitType, rate_limit_type::orders_1m);
    EXPECT_EQ(rl.interval, rate_limit_interval::minute);
    EXPECT_EQ(rl.intervalNum, 1);
    EXPECT_EQ(rl.limit, 1200);
    ASSERT_TRUE(rl.count.has_value());
    EXPECT_EQ(*rl.count, 42);

    auto json2 = to_json(rl);
    auto rl2 = parse_json<rate_limit>(json2);
    ASSERT_TRUE(rl2.count.has_value());
    EXPECT_EQ(*rl2.count, 42);
}

// ---------- price_level (JSON array format) ----------

TEST(JsonRoundTrip, PriceLevel)
{
    constexpr std::string_view input = R"(["50000.5","1.234"])";
    auto pl = parse_json<price_level>(input);

    EXPECT_EQ(pl.price, decimal_t("50000.5"));
    EXPECT_EQ(pl.quantity, decimal_t("1.234"));

    auto json2 = to_json(pl);
    auto pl2 = parse_json<price_level>(json2);
    EXPECT_EQ(pl2.price, pl.price);
    EXPECT_EQ(pl2.quantity, pl.quantity);
}

// ---------- listen_key_response ----------

TEST(JsonRoundTrip, ListenKeyResponse)
{
    constexpr std::string_view input = R"({"listenKey":"abc123"})";
    auto resp = parse_json<listen_key_response>(input);

    EXPECT_EQ(resp.listenKey, "abc123");

    auto json2 = to_json(resp);
    auto resp2 = parse_json<listen_key_response>(json2);
    EXPECT_EQ(resp2.listenKey, resp.listenKey);
}

// ---------- exchange_info_asset ----------

TEST(JsonRoundTrip, ExchangeInfoAssetWithOptional)
{
    constexpr std::string_view input =
        R"({"asset":"USDT","marginAvailable":true,"autoAssetExchange":"-0.00100000"})";
    auto a = parse_json<exchange_info_asset>(input);

    EXPECT_EQ(a.asset, "USDT");
    EXPECT_TRUE(a.marginAvailable);
    ASSERT_TRUE(a.autoAssetExchange.has_value());
    EXPECT_EQ(*a.autoAssetExchange, "-0.00100000");

    auto json2 = to_json(a);
    auto a2 = parse_json<exchange_info_asset>(json2);
    EXPECT_EQ(a2.asset, a.asset);
    EXPECT_EQ(a2.marginAvailable, a.marginAvailable);
    ASSERT_TRUE(a2.autoAssetExchange.has_value());
    EXPECT_EQ(*a2.autoAssetExchange, *a.autoAssetExchange);
}

TEST(JsonRoundTrip, ExchangeInfoAssetWithoutOptional)
{
    constexpr std::string_view input = R"({"asset":"BTC","marginAvailable":false})";
    auto a = parse_json<exchange_info_asset>(input);

    EXPECT_EQ(a.asset, "BTC");
    EXPECT_FALSE(a.marginAvailable);
    EXPECT_FALSE(a.autoAssetExchange.has_value());

    auto json2 = to_json(a);
    auto a2 = parse_json<exchange_info_asset>(json2);
    EXPECT_EQ(a2.asset, a.asset);
    EXPECT_FALSE(a2.autoAssetExchange.has_value());
}

// ============================================================================
// Part 2: JSON round-trips for trade types
// ============================================================================

TEST(JsonRoundTrip, OrderResponse)
{
    constexpr std::string_view input = R"({
        "clientOrderId": "testOrder",
        "cumQty": "0.0",
        "cumQuote": "0.0",
        "executedQty": "0.0",
        "orderId": 22542179,
        "avgPrice": "0.0",
        "origQty": "10.0",
        "price": "0.0",
        "reduceOnly": false,
        "side": "BUY",
        "positionSide": "SHORT",
        "status": "NEW",
        "stopPrice": "9300.0",
        "closePosition": false,
        "symbol": "BTCUSDT",
        "timeInForce": "GTC",
        "type": "TRAILING_STOP_MARKET",
        "origType": "TRAILING_STOP_MARKET",
        "activatePrice": "9020.0",
        "priceRate": "0.3",
        "updateTime": 1566818724722,
        "workingType": "CONTRACT_PRICE",
        "priceProtect": false,
        "priceMatch": "NONE",
        "selfTradePreventionMode": "EXPIRE_TAKER",
        "goodTillDate": 0
    })";
    auto resp = parse_json<order_response>(input);

    EXPECT_EQ(resp.clientOrderId, "testOrder");
    EXPECT_EQ(resp.orderId, 22542179ULL);
    EXPECT_EQ(resp.side, order_side::buy);
    EXPECT_EQ(resp.positionSide, position_side::short_side);
    EXPECT_EQ(resp.status, order_status::new_order);
    EXPECT_EQ(resp.symbol, "BTCUSDT");
    EXPECT_EQ(resp.timeInForce, time_in_force::gtc);
    EXPECT_EQ(resp.type, order_type::trailing_stop_market);
    EXPECT_EQ(resp.origType, order_type::trailing_stop_market);
    EXPECT_EQ(resp.origQty, decimal_t("10.0"));
    EXPECT_EQ(resp.stopPrice, decimal_t("9300.0"));
    EXPECT_FALSE(resp.reduceOnly);
    EXPECT_FALSE(resp.closePosition);

    ASSERT_TRUE(resp.activatePrice.has_value());
    EXPECT_EQ(*resp.activatePrice, decimal_t("9020.0"));
    ASSERT_TRUE(resp.priceRate.has_value());
    EXPECT_EQ(*resp.priceRate, decimal_t("0.3"));
    ASSERT_TRUE(resp.updateTime.has_value());
    EXPECT_EQ(*resp.updateTime, timestamp_ms_t{1566818724722ULL});
    ASSERT_TRUE(resp.workingType.has_value());
    EXPECT_EQ(*resp.workingType, working_type::contract_price);
    ASSERT_TRUE(resp.priceProtect.has_value());
    EXPECT_FALSE(*resp.priceProtect);
    ASSERT_TRUE(resp.priceMatch.has_value());
    EXPECT_EQ(*resp.priceMatch, price_match::none);
    ASSERT_TRUE(resp.selfTradePreventionMode.has_value());
    EXPECT_EQ(*resp.selfTradePreventionMode, stp_mode::expire_taker);

    // Round-trip.
    auto json2 = to_json(resp);
    auto resp2 = parse_json<order_response>(json2);
    EXPECT_EQ(resp2.orderId, resp.orderId);
    EXPECT_EQ(resp2.side, resp.side);
    EXPECT_EQ(resp2.status, resp.status);
    EXPECT_EQ(resp2.symbol, resp.symbol);
    EXPECT_EQ(resp2.type, resp.type);
}

// ============================================================================
// Part 3: to_query_map
// ============================================================================

// ---------- order_book_request ----------

TEST(ToQueryMap, OrderBookRequestWithLimit)
{
    order_book_request req{ .symbol = "BTCUSDT", .limit = 10 };
    auto m = binapi2::fapi::to_query_map(req);

    EXPECT_EQ(m.at("symbol"), "BTCUSDT");
    EXPECT_EQ(m.at("limit"), "10");
    EXPECT_EQ(m.size(), 2u);
}

TEST(ToQueryMap, OrderBookRequestWithoutLimit)
{
    order_book_request req{ .symbol = "BTCUSDT" };
    auto m = binapi2::fapi::to_query_map(req);

    EXPECT_EQ(m.at("symbol"), "BTCUSDT");
    EXPECT_EQ(m.count("limit"), 0u);
    EXPECT_EQ(m.size(), 1u);
}

// ---------- new_order_request with enum and decimal fields ----------

TEST(ToQueryMap, NewOrderRequestFull)
{
    new_order_request req{};
    req.symbol = "ETHUSDT";
    req.side = order_side::sell;
    req.type = order_type::limit;
    req.timeInForce = time_in_force::gtc;
    req.quantity = decimal_t("0.5");
    req.price = decimal_t("3000.0");

    auto m = binapi2::fapi::to_query_map(req);

    EXPECT_EQ(m.at("symbol"), "ETHUSDT");
    EXPECT_EQ(m.at("side"), "SELL");
    EXPECT_EQ(m.at("type"), "LIMIT");
    EXPECT_EQ(m.at("timeInForce"), "GTC");
    EXPECT_EQ(m.at("quantity"), "0.5");
    EXPECT_EQ(m.at("price"), "3000.0");

    // Optional fields that were not set should be absent.
    EXPECT_EQ(m.count("newClientOrderId"), 0u);
    EXPECT_EQ(m.count("stopPrice"), 0u);
    EXPECT_EQ(m.count("positionSide"), 0u);
    EXPECT_EQ(m.count("reduceOnly"), 0u);
    EXPECT_EQ(m.count("closePosition"), 0u);
    EXPECT_EQ(m.count("activationPrice"), 0u);
    EXPECT_EQ(m.count("callbackRate"), 0u);
    EXPECT_EQ(m.count("workingType"), 0u);
    EXPECT_EQ(m.count("priceProtect"), 0u);
    EXPECT_EQ(m.count("newOrderRespType"), 0u);
    EXPECT_EQ(m.count("priceMatch"), 0u);
    EXPECT_EQ(m.count("selfTradePreventionMode"), 0u);
    EXPECT_EQ(m.count("goodTillDate"), 0u);
}

// ---------- exchange_info_request ----------

TEST(ToQueryMap, ExchangeInfoRequestWithSymbol)
{
    exchange_info_request req{ .symbol = "BTCUSDT" };
    auto m = binapi2::fapi::to_query_map(req);

    EXPECT_EQ(m.at("symbol"), "BTCUSDT");
    EXPECT_EQ(m.size(), 1u);
}

TEST(ToQueryMap, ExchangeInfoRequestEmpty)
{
    exchange_info_request req{};
    auto m = binapi2::fapi::to_query_map(req);

    EXPECT_TRUE(m.empty());
}

} // anonymous namespace
