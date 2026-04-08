// SPDX-License-Identifier: Apache-2.0
//
// Parse the JSON response fixtures from compose/postman-mock/responses/
// and verify they deserialize into the corresponding binapi2 types.

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
// Single-object responses
// ---------------------------------------------------------------------------

TEST(ResponseParse, Ping)
{
    auto json = read_file(fixture("ping.json"));
    empty_response r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
}

TEST(ResponseParse, ServerTime)
{
    auto json = read_file(fixture("server_time.json"));
    server_time_response r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_NE(r.serverTime, timestamp_ms{});
}

TEST(ResponseParse, ExchangeInfo)
{
    auto json = read_file(fixture("exchange_info.json"));
    exchange_info_response r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_EQ(r.timezone, "UTC");
    EXPECT_EQ(r.futuresType, futures_type::u_margined);
    EXPECT_FALSE(r.rateLimits.empty());
    EXPECT_FALSE(r.symbols.empty());
}

TEST(ResponseParse, OrderBook)
{
    auto json = read_file(fixture("depth.json"));
    order_book_response r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_GT(r.lastUpdateId, 0ULL);
    EXPECT_FALSE(r.bids.empty());
    EXPECT_FALSE(r.asks.empty());
}

TEST(ResponseParse, ListenKey)
{
    auto json = read_file(fixture("listen_key.json"));
    listen_key_response r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.listenKey.empty());
}

TEST(ResponseParse, Order)
{
    auto json = read_file(fixture("order.json"));
    order_response r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_GT(r.orderId, 0ULL);
    EXPECT_FALSE(r.symbol.empty());
}

TEST(ResponseParse, BookTicker)
{
    auto json = read_file(fixture("ticker_book.json"));
    book_ticker r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_EQ(r.symbol, "BTCUSDT");
}

TEST(ResponseParse, PriceTicker)
{
    auto json = read_file(fixture("ticker_price.json"));
    price_ticker r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_EQ(r.symbol, "BTCUSDT");
}

TEST(ResponseParse, PremiumIndex)
{
    auto json = read_file(fixture("premium_index.json"));
    mark_price r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_EQ(r.symbol, "BTCUSDT");
}

TEST(ResponseParse, Account)
{
    auto json = read_file(fixture("account.json"));
    account_information r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
}

// ---------------------------------------------------------------------------
// Array responses
// ---------------------------------------------------------------------------

TEST(ResponseParse, Balances)
{
    auto json = read_file(fixture("balance.json"));
    std::vector<futures_account_balance> r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.empty());
    EXPECT_FALSE(r[0].asset.empty());
}

TEST(ResponseParse, Trades)
{
    auto json = read_file(fixture("trades.json"));
    std::vector<recent_trade> r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.empty());
}

TEST(ResponseParse, FundingRate)
{
    auto json = read_file(fixture("funding_rate.json"));
    std::vector<funding_rate_history_entry> r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(r[0].symbol, "BTCUSDT");
}

TEST(ResponseParse, Klines)
{
    auto json = read_file(fixture("klines.json"));
    std::vector<kline> r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.empty());
}

TEST(ResponseParse, OpenOrders)
{
    auto json = read_file(fixture("open_orders.json"));
    std::vector<order_response> r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.empty());
}

TEST(ResponseParse, PositionRisk)
{
    auto json = read_file(fixture("position_risk.json"));
    std::vector<position_risk> r;
    auto ec = glz::read_json(r, json);
    EXPECT_FALSE(ec) << glz::format_error(ec, json);
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(r[0].symbol, "BTCUSDT");
}

} // namespace
