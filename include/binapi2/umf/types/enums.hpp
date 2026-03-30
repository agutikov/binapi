#pragma once

#include <string>

namespace binapi2::umf::types {

enum class security_type {
    none,
    market_data,
    user_stream,
    user_data,
    trade,
};

enum class order_side {
    buy,
    sell,
};

enum class order_type {
    limit,
    market,
    stop,
    stop_market,
    take_profit,
    take_profit_market,
};

enum class time_in_force {
    gtc,
    ioc,
    fok,
    gtx,
};

[[nodiscard]] inline std::string to_string(security_type value) {
    switch (value) {
        case security_type::none: return "none";
        case security_type::market_data: return "market_data";
        case security_type::user_stream: return "user_stream";
        case security_type::user_data: return "user_data";
        case security_type::trade: return "trade";
    }
    return "none";
}

[[nodiscard]] inline std::string to_string(order_side value) {
    switch (value) {
        case order_side::buy: return "BUY";
        case order_side::sell: return "SELL";
    }
    return "BUY";
}

[[nodiscard]] inline std::string to_string(order_type value) {
    switch (value) {
        case order_type::limit: return "LIMIT";
        case order_type::market: return "MARKET";
        case order_type::stop: return "STOP";
        case order_type::stop_market: return "STOP_MARKET";
        case order_type::take_profit: return "TAKE_PROFIT";
        case order_type::take_profit_market: return "TAKE_PROFIT_MARKET";
    }
    return "LIMIT";
}

[[nodiscard]] inline std::string to_string(time_in_force value) {
    switch (value) {
        case time_in_force::gtc: return "GTC";
        case time_in_force::ioc: return "IOC";
        case time_in_force::fok: return "FOK";
        case time_in_force::gtx: return "GTX";
    }
    return "GTC";
}

} // namespace binapi2::umf::types
