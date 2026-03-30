#pragma once

#include <string>

namespace binapi2::umf::streams {

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

} // namespace binapi2::umf::streams
