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

enum class kline_interval {
    m1,
    m3,
    m5,
    m15,
    m30,
    h1,
    h2,
    h4,
    h6,
    h8,
    h12,
    d1,
    d3,
    w1,
    mo1,
};

enum class contract_type {
    perpetual,
    current_quarter,
    next_quarter,
    tradifi_perpetual,
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

[[nodiscard]] inline std::string to_string(kline_interval value) {
    switch (value) {
        case kline_interval::m1: return "1m";
        case kline_interval::m3: return "3m";
        case kline_interval::m5: return "5m";
        case kline_interval::m15: return "15m";
        case kline_interval::m30: return "30m";
        case kline_interval::h1: return "1h";
        case kline_interval::h2: return "2h";
        case kline_interval::h4: return "4h";
        case kline_interval::h6: return "6h";
        case kline_interval::h8: return "8h";
        case kline_interval::h12: return "12h";
        case kline_interval::d1: return "1d";
        case kline_interval::d3: return "3d";
        case kline_interval::w1: return "1w";
        case kline_interval::mo1: return "1M";
    }
    return "1m";
}

[[nodiscard]] inline std::string to_string(contract_type value) {
    switch (value) {
        case contract_type::perpetual: return "PERPETUAL";
        case contract_type::current_quarter: return "CURRENT_QUARTER";
        case contract_type::next_quarter: return "NEXT_QUARTER";
        case contract_type::tradifi_perpetual: return "TRADIFI_PERPETUAL";
    }
    return "PERPETUAL";
}

} // namespace binapi2::umf::types
