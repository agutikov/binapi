// SPDX-License-Identifier: Apache-2.0
//
// Parse the JSON response fixtures from compose/postman-mock/responses/
// and verify they deserialize into the corresponding binapi2 types.

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <gtest/gtest.h>
#include <glaze/glaze.hpp>

#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string read_file(const std::string& path)
{
    std::ifstream f(path);
    EXPECT_TRUE(f.good()) << "Cannot open: " << path;
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string fixture(const char* name)
{
    return std::string(RESPONSES_PATH) + "/" + name;
}

using namespace binapi2::fapi::types;

// ---------------------------------------------------------------------------
// Helper: parse fixture through the production decode_response path.
// ---------------------------------------------------------------------------

template<typename T>
binapi2::fapi::result<T> decode_fixture(const char* name) {
    binapi2::fapi::transport::http_response resp;
    resp.status = 200;
    resp.body = read_file(fixture(name));
    return binapi2::fapi::detail::decode_response<T>(resp);
}

// ---------------------------------------------------------------------------
// Single-object responses
// ---------------------------------------------------------------------------

TEST(ResponseParse, Ping)
{
    auto r = decode_fixture<empty_response_t>("ping.json");
    EXPECT_TRUE(r) << r.err.message;
}

TEST(ResponseParse, ServerTime)
{
    auto r = decode_fixture<server_time_response_t>("server_time.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_NE(r->serverTime, timestamp_ms_t{});
}

TEST(ResponseParse, ExchangeInfo)
{
    auto r = decode_fixture<exchange_info_response_t>("exchange_info.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->timezone, "UTC");
    EXPECT_EQ(r->futuresType, futures_type_t::u_margined);
    EXPECT_FALSE(r->rateLimits.empty());
    EXPECT_FALSE(r->symbols.empty());
}

TEST(ResponseParse, OrderBook)
{
    auto r = decode_fixture<order_book_response_t>("depth.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_GT(r->lastUpdateId, 0ULL);
    EXPECT_FALSE(r->bids.empty());
    EXPECT_FALSE(r->asks.empty());
}

TEST(ResponseParse, ListenKey)
{
    auto r = decode_fixture<listen_key_response_t>("listen_key.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->listenKey.empty());
}

TEST(ResponseParse, Order)
{
    auto r = decode_fixture<order_response_t>("order.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_GT(r->orderId, 0ULL);
    EXPECT_FALSE(r->symbol.empty());
}

TEST(ResponseParse, BookTicker)
{
    auto r = decode_fixture<book_ticker_t>("ticker_book.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->symbol, "BTCUSDT");
}

TEST(ResponseParse, PriceTicker)
{
    auto r = decode_fixture<price_ticker_t>("ticker_price.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->symbol, "BTCUSDT");
}

TEST(ResponseParse, PremiumIndex)
{
    auto r = decode_fixture<mark_price_t>("premium_index.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->symbol, "BTCUSDT");
}

TEST(ResponseParse, Account)
{
    auto r = decode_fixture<account_information_t>("account.json");
    EXPECT_TRUE(r) << r.err.message;
}

// ---------------------------------------------------------------------------
// Array responses
// ---------------------------------------------------------------------------

TEST(ResponseParse, Balances)
{
    auto r = decode_fixture<std::vector<futures_account_balance_t>>("balance.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->empty());
    EXPECT_FALSE((*r)[0].asset.empty());
}

TEST(ResponseParse, Trades)
{
    auto r = decode_fixture<std::vector<recent_trade_t>>("trades.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->empty());
}

TEST(ResponseParse, FundingRate)
{
    auto r = decode_fixture<std::vector<funding_rate_history_entry_t>>("funding_rate.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->empty());
    EXPECT_EQ((*r)[0].symbol, "BTCUSDT");
}

TEST(ResponseParse, Klines)
{
    auto r = decode_fixture<std::vector<kline_t>>("klines.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->empty());
}

TEST(ResponseParse, OpenOrders)
{
    auto r = decode_fixture<std::vector<order_response_t>>("open_orders.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->empty());
}

TEST(ResponseParse, PositionRisk)
{
    auto r = decode_fixture<std::vector<position_risk_t>>("position_risk.json");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_FALSE(r->empty());
    EXPECT_EQ((*r)[0].symbol, "BTCUSDT");
}

// ---------------------------------------------------------------------------
// Unknown/extra fields — must be silently ignored (Binance adds fields often)
// ---------------------------------------------------------------------------

/// Helper: parse JSON via decode_response (the real production path).
template<typename T>
binapi2::fapi::result<T> decode(int status, const std::string& body) {
    binapi2::fapi::transport::http_response resp;
    resp.status = status;
    resp.body = body;
    return binapi2::fapi::detail::decode_response<T>(resp);
}

TEST(ExtraFields, ServerTime)
{
    auto r = decode<server_time_response_t>(200,
        R"({"serverTime":1700000000000,"newField":"surprise","anotherOne":42})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->serverTime, timestamp_ms_t{1700000000000});
}

TEST(ExtraFields, ListenKey)
{
    auto r = decode<listen_key_response_t>(200,
        R"({"listenKey":"abc123","extraBool":true})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->listenKey, "abc123");
}

TEST(ExtraFields, BookTicker)
{
    auto r = decode<book_ticker_t>(200,
        R"({"symbol":"BTCUSDT","bidPrice":"50000","bidQty":"1","askPrice":"50001","askQty":"2","time":1700000000000,"lastUpdateId":42,"unknownField":"x"})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->symbol, "BTCUSDT");
}

TEST(ExtraFields, PriceTicker)
{
    auto r = decode<price_ticker_t>(200,
        R"({"symbol":"ETHUSDT","price":"3000","time":1700000000000,"futureField":{"nested":true}})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->symbol, "ETHUSDT");
}

TEST(ExtraFields, OrderResponse)
{
    // Minimal valid order response with an extra field injected.
    auto r = decode<order_response_t>(200,
        R"({"orderId":123,"symbol":"BTCUSDT","status":"NEW","clientOrderId":"x",)"
        R"("price":"0","avgPrice":"0","origQty":"1","executedQty":"0","cumQty":"0","cumQuote":"0",)"
        R"("timeInForce":"GTC","type":"LIMIT","reduceOnly":false,"closePosition":false,)"
        R"("side":"BUY","positionSide":"BOTH","stopPrice":"0",)"
        R"("origType":"LIMIT",)"
        R"("newFieldFromBinance":999})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->orderId, 123ULL);
}

TEST(ExtraFields, ArrayResponse)
{
    // Vector of objects, each with an extra field.
    auto r = decode<std::vector<price_ticker_t>>(200,
        R"([{"symbol":"BTCUSDT","price":"50000","time":1700000000000,"extra":1},)"
        R"({"symbol":"ETHUSDT","price":"3000","time":1700000000000,"extra":2}])");
    ASSERT_TRUE(r) << r.err.message;
    ASSERT_EQ(r->size(), 2u);
    EXPECT_EQ((*r)[0].symbol, "BTCUSDT");
    EXPECT_EQ((*r)[1].symbol, "ETHUSDT");
}

TEST(ExtraFields, ErrorDocument)
{
    // Binance error response with extra fields should still parse the error.
    auto r = decode<server_time_response_t>(400,
        R"({"code":-1021,"msg":"Timestamp outside recv window","extra":"data"})");
    ASSERT_FALSE(r);
    EXPECT_EQ(r.err.code, binapi2::fapi::error_code::binance);
    EXPECT_EQ(r.err.binance_code, -1021);
}

// ---------------------------------------------------------------------------
// Missing optional fields — must parse successfully with nullopt defaults
// ---------------------------------------------------------------------------

TEST(MissingOptional, OrderResponseMinimal)
{
    // All std::optional fields omitted — should parse fine.
    auto r = decode<order_response_t>(200,
        R"({"orderId":1,"symbol":"BTCUSDT","status":"NEW","clientOrderId":"x",)"
        R"("price":"0","avgPrice":"0","origQty":"1","executedQty":"0","cumQty":"0","cumQuote":"0",)"
        R"("timeInForce":"GTC","type":"LIMIT","reduceOnly":false,"closePosition":false,)"
        R"("side":"BUY","positionSide":"BOTH","stopPrice":"0","origType":"LIMIT"})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->orderId, 1ULL);
    EXPECT_FALSE(r->workingType.has_value());
    EXPECT_FALSE(r->priceProtect.has_value());
    EXPECT_FALSE(r->priceMatch.has_value());
    EXPECT_FALSE(r->selfTradePreventionMode.has_value());
    EXPECT_FALSE(r->goodTillDate.has_value());
    EXPECT_FALSE(r->activatePrice.has_value());
    EXPECT_FALSE(r->priceRate.has_value());
    EXPECT_FALSE(r->time.has_value());
    EXPECT_FALSE(r->updateTime.has_value());
}

TEST(MissingOptional, PriceTickerNoOptionals)
{
    // price_ticker_t has no optional fields, but this confirms a complete
    // minimal object parses.
    auto r = decode<price_ticker_t>(200,
        R"({"symbol":"ETHUSDT","price":"3000","time":1700000000000})");
    ASSERT_TRUE(r) << r.err.message;
    EXPECT_EQ(r->symbol, "ETHUSDT");
}

// ---------------------------------------------------------------------------
// Missing required fields — must fail with error_code::json
// ---------------------------------------------------------------------------

TEST(MissingRequired, ServerTimeMissingField)
{
    // serverTime is required (not std::optional) — empty object must fail.
    auto r = decode<server_time_response_t>(200, R"({})");
    EXPECT_FALSE(r);
    EXPECT_EQ(r.err.code, binapi2::fapi::error_code::json);
}

TEST(MissingRequired, ListenKeyMissingField)
{
    auto r = decode<listen_key_response_t>(200, R"({})");
    EXPECT_FALSE(r);
    EXPECT_EQ(r.err.code, binapi2::fapi::error_code::json);
}

TEST(MissingRequired, BookTickerMissingSymbol)
{
    // symbol is required — omit it, keep everything else.
    auto r = decode<book_ticker_t>(200,
        R"({"bidPrice":"1","bidQty":"1","askPrice":"1","askQty":"1","time":0,"lastUpdateId":0})");
    EXPECT_FALSE(r);
    EXPECT_EQ(r.err.code, binapi2::fapi::error_code::json);
}

TEST(MissingRequired, OrderResponseMissingOrderId)
{
    // orderId is required — omit it.
    auto r = decode<order_response_t>(200,
        R"({"symbol":"BTCUSDT","status":"NEW","clientOrderId":"x",)"
        R"("price":"0","avgPrice":"0","origQty":"1","executedQty":"0","cumQty":"0","cumQuote":"0",)"
        R"("timeInForce":"GTC","type":"LIMIT","reduceOnly":false,"closePosition":false,)"
        R"("side":"BUY","positionSide":"BOTH","stopPrice":"0","origType":"LIMIT"})");
    EXPECT_FALSE(r);
    EXPECT_EQ(r.err.code, binapi2::fapi::error_code::json);
}

TEST(MissingRequired, PriceTickerMissingPrice)
{
    auto r = decode<price_ticker_t>(200,
        R"({"symbol":"BTCUSDT","time":1700000000000})");
    EXPECT_FALSE(r);
    EXPECT_EQ(r.err.code, binapi2::fapi::error_code::json);
}

} // namespace
