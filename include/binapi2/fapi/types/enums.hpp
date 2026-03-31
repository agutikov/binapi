// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <string>

namespace binapi2::fapi::types {

enum class security_type
{
    none,
    market_data,
    user_stream,
    user_data,
    trade,
};

enum class order_side
{
    buy,
    sell,
};

enum class order_type
{
    limit,
    market,
    stop,
    stop_market,
    take_profit,
    take_profit_market,
    trailing_stop_market,
};

enum class time_in_force
{
    gtc,
    ioc,
    fok,
    gtx,
    gtd,
};

enum class kline_interval
{
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

enum class position_side
{
    both,
    long_side,
    short_side,
};

enum class working_type
{
    mark_price,
    contract_price,
};

enum class response_type
{
    ack,
    result,
};

enum class margin_type
{
    isolated,
    crossed,
};

enum class contract_type
{
    perpetual,
    current_month,
    next_month,
    current_quarter,
    next_quarter,
    perpetual_delivering,
    tradifi_perpetual,
};

enum class contract_status
{
    pending_trading,
    trading,
    pre_delivering,
    delivering,
    delivered,
    pre_settle,
    settling,
    close,
};

enum class order_status
{
    new_order,
    partially_filled,
    filled,
    canceled,
    rejected,
    expired,
    expired_in_match,
};

enum class stp_mode
{
    expire_taker,
    expire_both,
    expire_maker,
};

enum class price_match
{
    none,
    opponent,
    opponent_5,
    opponent_10,
    opponent_20,
    queue,
    queue_5,
    queue_10,
    queue_20,
};

enum class income_type
{
    transfer,
    welcome_bonus,
    realized_pnl,
    funding_fee,
    commission,
    insurance_clear,
    referral_kickback,
    commission_rebate,
    api_rebate,
    contest_reward,
    cross_collateral_transfer,
    options_premium_fee,
    options_settle_profit,
    internal_transfer,
    auto_exchange,
    delivered_settelment,
    coin_swap_deposit,
    coin_swap_withdraw,
    position_limit_increase_fee,
};

enum class futures_data_period
{
    m5,
    m15,
    m30,
    h1,
    h2,
    h4,
    h6,
    h12,
    d1,
};

[[nodiscard]] inline std::string
to_string(security_type value)
{
    switch (value) {
        case security_type::none:
            return "none";
        case security_type::market_data:
            return "market_data";
        case security_type::user_stream:
            return "user_stream";
        case security_type::user_data:
            return "user_data";
        case security_type::trade:
            return "trade";
    }
    return "none";
}

[[nodiscard]] inline std::string
to_string(order_side value)
{
    switch (value) {
        case order_side::buy:
            return "BUY";
        case order_side::sell:
            return "SELL";
    }
    return "BUY";
}

[[nodiscard]] inline std::string
to_string(order_type value)
{
    switch (value) {
        case order_type::limit:
            return "LIMIT";
        case order_type::market:
            return "MARKET";
        case order_type::stop:
            return "STOP";
        case order_type::stop_market:
            return "STOP_MARKET";
        case order_type::take_profit:
            return "TAKE_PROFIT";
        case order_type::take_profit_market:
            return "TAKE_PROFIT_MARKET";
        case order_type::trailing_stop_market:
            return "TRAILING_STOP_MARKET";
    }
    return "LIMIT";
}

[[nodiscard]] inline std::string
to_string(time_in_force value)
{
    switch (value) {
        case time_in_force::gtc:
            return "GTC";
        case time_in_force::ioc:
            return "IOC";
        case time_in_force::fok:
            return "FOK";
        case time_in_force::gtx:
            return "GTX";
        case time_in_force::gtd:
            return "GTD";
    }
    return "GTC";
}

[[nodiscard]] inline std::string
to_string(kline_interval value)
{
    switch (value) {
        case kline_interval::m1:
            return "1m";
        case kline_interval::m3:
            return "3m";
        case kline_interval::m5:
            return "5m";
        case kline_interval::m15:
            return "15m";
        case kline_interval::m30:
            return "30m";
        case kline_interval::h1:
            return "1h";
        case kline_interval::h2:
            return "2h";
        case kline_interval::h4:
            return "4h";
        case kline_interval::h6:
            return "6h";
        case kline_interval::h8:
            return "8h";
        case kline_interval::h12:
            return "12h";
        case kline_interval::d1:
            return "1d";
        case kline_interval::d3:
            return "3d";
        case kline_interval::w1:
            return "1w";
        case kline_interval::mo1:
            return "1M";
    }
    return "1m";
}

[[nodiscard]] inline std::string
to_string(position_side value)
{
    switch (value) {
        case position_side::both:
            return "BOTH";
        case position_side::long_side:
            return "LONG";
        case position_side::short_side:
            return "SHORT";
    }
    return "BOTH";
}

[[nodiscard]] inline std::string
to_string(working_type value)
{
    switch (value) {
        case working_type::mark_price:
            return "MARK_PRICE";
        case working_type::contract_price:
            return "CONTRACT_PRICE";
    }
    return "MARK_PRICE";
}

[[nodiscard]] inline std::string
to_string(response_type value)
{
    switch (value) {
        case response_type::ack:
            return "ACK";
        case response_type::result:
            return "RESULT";
    }
    return "ACK";
}

[[nodiscard]] inline std::string
to_string(margin_type value)
{
    switch (value) {
        case margin_type::isolated:
            return "ISOLATED";
        case margin_type::crossed:
            return "CROSSED";
    }
    return "ISOLATED";
}

[[nodiscard]] inline std::string
to_string(contract_type value)
{
    switch (value) {
        case contract_type::perpetual:
            return "PERPETUAL";
        case contract_type::current_month:
            return "CURRENT_MONTH";
        case contract_type::next_month:
            return "NEXT_MONTH";
        case contract_type::current_quarter:
            return "CURRENT_QUARTER";
        case contract_type::next_quarter:
            return "NEXT_QUARTER";
        case contract_type::perpetual_delivering:
            return "PERPETUAL_DELIVERING";
        case contract_type::tradifi_perpetual:
            return "TRADIFI_PERPETUAL";
    }
    return "PERPETUAL";
}

[[nodiscard]] inline std::string
to_string(contract_status value)
{
    switch (value) {
        case contract_status::pending_trading:
            return "PENDING_TRADING";
        case contract_status::trading:
            return "TRADING";
        case contract_status::pre_delivering:
            return "PRE_DELIVERING";
        case contract_status::delivering:
            return "DELIVERING";
        case contract_status::delivered:
            return "DELIVERED";
        case contract_status::pre_settle:
            return "PRE_SETTLE";
        case contract_status::settling:
            return "SETTLING";
        case contract_status::close:
            return "CLOSE";
    }
    return "TRADING";
}

[[nodiscard]] inline std::string
to_string(order_status value)
{
    switch (value) {
        case order_status::new_order:
            return "NEW";
        case order_status::partially_filled:
            return "PARTIALLY_FILLED";
        case order_status::filled:
            return "FILLED";
        case order_status::canceled:
            return "CANCELED";
        case order_status::rejected:
            return "REJECTED";
        case order_status::expired:
            return "EXPIRED";
        case order_status::expired_in_match:
            return "EXPIRED_IN_MATCH";
    }
    return "NEW";
}

[[nodiscard]] inline std::string
to_string(stp_mode value)
{
    switch (value) {
        case stp_mode::expire_taker:
            return "EXPIRE_TAKER";
        case stp_mode::expire_both:
            return "EXPIRE_BOTH";
        case stp_mode::expire_maker:
            return "EXPIRE_MAKER";
    }
    return "EXPIRE_TAKER";
}

[[nodiscard]] inline std::string
to_string(price_match value)
{
    switch (value) {
        case price_match::none:
            return "NONE";
        case price_match::opponent:
            return "OPPONENT";
        case price_match::opponent_5:
            return "OPPONENT_5";
        case price_match::opponent_10:
            return "OPPONENT_10";
        case price_match::opponent_20:
            return "OPPONENT_20";
        case price_match::queue:
            return "QUEUE";
        case price_match::queue_5:
            return "QUEUE_5";
        case price_match::queue_10:
            return "QUEUE_10";
        case price_match::queue_20:
            return "QUEUE_20";
    }
    return "NONE";
}

[[nodiscard]] inline std::string
to_string(income_type value)
{
    switch (value) {
        case income_type::transfer:
            return "TRANSFER";
        case income_type::welcome_bonus:
            return "WELCOME_BONUS";
        case income_type::realized_pnl:
            return "REALIZED_PNL";
        case income_type::funding_fee:
            return "FUNDING_FEE";
        case income_type::commission:
            return "COMMISSION";
        case income_type::insurance_clear:
            return "INSURANCE_CLEAR";
        case income_type::referral_kickback:
            return "REFERRAL_KICKBACK";
        case income_type::commission_rebate:
            return "COMMISSION_REBATE";
        case income_type::api_rebate:
            return "API_REBATE";
        case income_type::contest_reward:
            return "CONTEST_REWARD";
        case income_type::cross_collateral_transfer:
            return "CROSS_COLLATERAL_TRANSFER";
        case income_type::options_premium_fee:
            return "OPTIONS_PREMIUM_FEE";
        case income_type::options_settle_profit:
            return "OPTIONS_SETTLE_PROFIT";
        case income_type::internal_transfer:
            return "INTERNAL_TRANSFER";
        case income_type::auto_exchange:
            return "AUTO_EXCHANGE";
        case income_type::delivered_settelment:
            return "DELIVERED_SETTELMENT";
        case income_type::coin_swap_deposit:
            return "COIN_SWAP_DEPOSIT";
        case income_type::coin_swap_withdraw:
            return "COIN_SWAP_WITHDRAW";
        case income_type::position_limit_increase_fee:
            return "POSITION_LIMIT_INCREASE_FEE";
    }
    return "TRANSFER";
}

[[nodiscard]] inline std::string
to_string(futures_data_period value)
{
    switch (value) {
        case futures_data_period::m5:
            return "5m";
        case futures_data_period::m15:
            return "15m";
        case futures_data_period::m30:
            return "30m";
        case futures_data_period::h1:
            return "1h";
        case futures_data_period::h2:
            return "2h";
        case futures_data_period::h4:
            return "4h";
        case futures_data_period::h6:
            return "6h";
        case futures_data_period::h12:
            return "12h";
        case futures_data_period::d1:
            return "1d";
    }
    return "5m";
}

} // namespace binapi2::fapi::types
