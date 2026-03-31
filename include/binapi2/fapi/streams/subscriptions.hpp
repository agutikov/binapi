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

struct ticker_subscription
{
    std::string symbol{};
};

struct kline_subscription
{
    std::string symbol{};
    types::kline_interval interval{ types::kline_interval::m1 };
};

} // namespace binapi2::fapi::streams
