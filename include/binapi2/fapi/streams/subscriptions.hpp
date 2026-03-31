// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/enums.hpp>

#include <string>

namespace binapi2::fapi::streams {

struct book_ticker_subscription
{
    std::string symbol{};
};

struct aggregate_trade_subscription
{
    std::string symbol{};
};

struct mark_price_subscription
{
    std::string symbol{};
    bool every_1s{ false };
};

struct diff_book_depth_subscription
{
    std::string symbol{};
    std::string speed{ "100ms" };
};

struct mini_ticker_subscription
{
    std::string symbol{};
};

struct all_market_mini_ticker_subscription
{
};

struct ticker_subscription
{
    std::string symbol{};
};

struct all_market_ticker_subscription
{
};

struct all_book_ticker_subscription
{
};

struct liquidation_order_subscription
{
    std::string symbol{};
};

struct all_market_liquidation_order_subscription
{
};

struct partial_book_depth_subscription
{
    std::string symbol{};
    int levels{ 5 };
    std::string speed{ "250ms" };
};

struct kline_subscription
{
    std::string symbol{};
    types::kline_interval interval{ types::kline_interval::m1 };
};

struct continuous_contract_kline_subscription
{
    std::string pair{};
    types::contract_type contract_type{ types::contract_type::perpetual };
    types::kline_interval interval{ types::kline_interval::m1 };
};

} // namespace binapi2::fapi::streams
