// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file enums.hpp
/// @brief Enumeration types for the Binance USD-M Futures API.
///
/// Each enum class maps to a Binance API constant set. The corresponding
/// to_string() overload converts each enumerator to the exact API wire-format
/// string (e.g. order_side::buy -> "BUY").

#pragma once

#include <glaze/glaze.hpp>

#include <string>

namespace binapi2::fapi::types {

/// API endpoint security classification. Determines what credentials
/// are required to call an endpoint.
enum class security_type
{
    none,
    market_data,
    user_stream,
    user_data,
    trade,
};

/// Order side: buy or sell.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_side
{
    buy,
    sell,
};

/// Order type: LIMIT, MARKET, and conditional order variants (STOP, TAKE_PROFIT, etc.).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_type
{
    limit,
    market,
    stop,              ///< Stop-limit order (requires price + stopPrice).
    stop_market,       ///< Stop-market order (requires stopPrice only).
    take_profit,       ///< Take-profit limit order.
    take_profit_market,///< Take-profit market order.
    trailing_stop_market,
};

/// Time-in-force policy for order lifetime.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class time_in_force
{
    gtc,  ///< Good Till Cancel.
    ioc,  ///< Immediate Or Cancel.
    fok,  ///< Fill Or Kill.
    gtx,  ///< Good Till Crossing (post-only).
    gtd,  ///< Good Till Date.
    rpi,  ///< Retail Price Improvement.
};

/// Kline/candlestick interval. Prefixed by unit: m=minutes, h=hours, d=days, w=weeks, mo=months.
/// to_string() produces the API format (e.g. m1 -> "1m", mo1 -> "1M").
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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

/// Position side for hedge mode. "both" is used in one-way mode.
/// long_side/short_side map to API strings "LONG"/"SHORT".
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class position_side
{
    both,
    long_side,   ///< Maps to "LONG" on the wire (avoids C++ keyword).
    short_side,  ///< Maps to "SHORT" on the wire (avoids C++ keyword).
};

/// Price type used for triggering conditional orders.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class working_type
{
    mark_price,
    contract_price,
};

/// Controls the detail level of order placement responses.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class response_type
{
    ack,
    result,
};

/// Position margin mode: isolated or cross (crossed).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class margin_type
{
    isolated,
    crossed,
};

/// Futures contract delivery type (perpetual, quarterly, etc.).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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

/// Trading status of a futures contract through its lifecycle.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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

/// Order execution status. Note: new_order maps to API string "NEW"
/// (renamed to avoid collision with C++ keywords/macros).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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

/// Self-Trade Prevention mode. Controls which side is expired when
/// an order would match against another order from the same account.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class stp_mode
{
    expire_taker,
    expire_both,
    expire_maker,
};

/// Price match mode for order placement. Determines how the order price
/// is derived relative to the current order book (opponent/queue side,
/// with optional tick offset).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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

/// Classification of account income/transaction entries returned by
/// the income history endpoint.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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

/// Aggregation period for futures statistics endpoints (open interest stats,
/// long/short ratio, taker volume, basis). to_string() produces API format
/// (e.g. m5 -> "5m").
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
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
        case time_in_force::rpi:
            return "RPI";
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

/// Execution type in order update events.
enum class execution_type
{
    new_order,
    partial_fill,
    fill,
    canceled,
    rejected,
    expired,
    trade,
};

[[nodiscard]] inline std::string
to_string(execution_type value)
{
    switch (value) {
        case execution_type::new_order: return "NEW";
        case execution_type::partial_fill: return "PARTIAL_FILL";
        case execution_type::fill: return "FILL";
        case execution_type::canceled: return "CANCELED";
        case execution_type::rejected: return "REJECTED";
        case execution_type::expired: return "EXPIRED";
        case execution_type::trade: return "TRADE";
    }
    return "NEW";
}

/// Rate limit type from exchange info.
enum class rate_limit_type
{
    request_weight,
    orders_1s,
    orders_1m,
    orders_1h,
    orders_1d,
};

[[nodiscard]] inline std::string
to_string(rate_limit_type value)
{
    switch (value) {
        case rate_limit_type::request_weight: return "REQUEST_WEIGHT";
        case rate_limit_type::orders_1s: return "ORDERS_1S";
        case rate_limit_type::orders_1m: return "ORDERS_1M";
        case rate_limit_type::orders_1h: return "ORDERS_1H";
        case rate_limit_type::orders_1d: return "ORDERS_1D";
    }
    return "REQUEST_WEIGHT";
}

/// Rate limit interval unit.
enum class rate_limit_interval
{
    second,
    minute,
    hour,
    day,
};

[[nodiscard]] inline std::string
to_string(rate_limit_interval value)
{
    switch (value) {
        case rate_limit_interval::second: return "SECOND";
        case rate_limit_interval::minute: return "MINUTE";
        case rate_limit_interval::hour: return "HOUR";
        case rate_limit_interval::day: return "DAY";
    }
    return "SECOND";
}

/// Auto-close type for forced liquidation/ADL orders.
enum class auto_close_type
{
    liquidation,
    adl,
};

[[nodiscard]] inline std::string
to_string(auto_close_type value)
{
    switch (value) {
        case auto_close_type::liquidation: return "LIQUIDATION";
        case auto_close_type::adl: return "ADL";
    }
    return "LIQUIDATION";
}

/// Isolated margin delta direction.
enum class delta_type
{
    add,
    reduce,
};

[[nodiscard]] inline std::string
to_string(delta_type value)
{
    switch (value) {
        case delta_type::add: return "1";
        case delta_type::reduce: return "2";
    }
    return "1";
}

/// Algo order type.
enum class algo_type
{
    twap,
    vp,
};

[[nodiscard]] inline std::string
to_string(algo_type value)
{
    switch (value) {
        case algo_type::twap: return "TWAP";
        case algo_type::vp: return "VP";
    }
    return "TWAP";
}

/// Algo order status.
enum class algo_status
{
    working,
    cancelled,
    rejected,
    expired,
    triggered,
};

[[nodiscard]] inline std::string
to_string(algo_status value)
{
    switch (value) {
        case algo_status::working: return "WORKING";
        case algo_status::cancelled: return "CANCELLED";
        case algo_status::rejected: return "REJECTED";
        case algo_status::expired: return "EXPIRED";
        case algo_status::triggered: return "TRIGGERED";
    }
    return "WORKING";
}

} // namespace binapi2::fapi::types

// ---------------------------------------------------------------------------
// Glaze enum metadata — maps wire-format strings to enum values for JSON
// deserialization. Each specialization uses glz::enumerate("WIRE_NAME", value).
// ---------------------------------------------------------------------------

template<>
struct glz::meta<binapi2::fapi::types::order_side>
{
    using enum binapi2::fapi::types::order_side;
    static constexpr auto value = enumerate("BUY", buy, "SELL", sell);
};

template<>
struct glz::meta<binapi2::fapi::types::order_type>
{
    using enum binapi2::fapi::types::order_type;
    static constexpr auto value = enumerate(
        "LIMIT", limit, "MARKET", market, "STOP", stop, "STOP_MARKET", stop_market,
        "TAKE_PROFIT", take_profit, "TAKE_PROFIT_MARKET", take_profit_market,
        "TRAILING_STOP_MARKET", trailing_stop_market);
};

template<>
struct glz::meta<binapi2::fapi::types::time_in_force>
{
    using enum binapi2::fapi::types::time_in_force;
    static constexpr auto value = enumerate("GTC", gtc, "IOC", ioc, "FOK", fok, "GTX", gtx, "GTD", gtd, "RPI", rpi);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_interval>
{
    using enum binapi2::fapi::types::kline_interval;
    static constexpr auto value = enumerate(
        "1m", m1, "3m", m3, "5m", m5, "15m", m15, "30m", m30,
        "1h", h1, "2h", h2, "4h", h4, "6h", h6, "8h", h8, "12h", h12,
        "1d", d1, "3d", d3, "1w", w1, "1M", mo1);
};

template<>
struct glz::meta<binapi2::fapi::types::position_side>
{
    using enum binapi2::fapi::types::position_side;
    static constexpr auto value = enumerate("BOTH", both, "LONG", long_side, "SHORT", short_side);
};

template<>
struct glz::meta<binapi2::fapi::types::working_type>
{
    using enum binapi2::fapi::types::working_type;
    static constexpr auto value = enumerate("MARK_PRICE", mark_price, "CONTRACT_PRICE", contract_price);
};

template<>
struct glz::meta<binapi2::fapi::types::response_type>
{
    using enum binapi2::fapi::types::response_type;
    static constexpr auto value = enumerate("ACK", ack, "RESULT", result);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_type>
{
    using enum binapi2::fapi::types::margin_type;
    static constexpr auto value = enumerate("ISOLATED", isolated, "CROSSED", crossed, "isolated", isolated, "cross", crossed);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_type>
{
    using enum binapi2::fapi::types::contract_type;
    static constexpr auto value = enumerate(
        "PERPETUAL", perpetual, "CURRENT_MONTH", current_month, "NEXT_MONTH", next_month,
        "CURRENT_QUARTER", current_quarter, "NEXT_QUARTER", next_quarter,
        "PERPETUAL_DELIVERING", perpetual_delivering, "TRADIFI_PERPETUAL", tradifi_perpetual);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_status>
{
    using enum binapi2::fapi::types::contract_status;
    static constexpr auto value = enumerate(
        "PENDING_TRADING", pending_trading, "TRADING", trading, "PRE_DELIVERING", pre_delivering,
        "DELIVERING", delivering, "DELIVERED", delivered, "PRE_SETTLE", pre_settle,
        "SETTLING", settling, "CLOSE", close);
};

template<>
struct glz::meta<binapi2::fapi::types::order_status>
{
    using enum binapi2::fapi::types::order_status;
    static constexpr auto value = enumerate(
        "NEW", new_order, "PARTIALLY_FILLED", partially_filled, "FILLED", filled,
        "CANCELED", canceled, "REJECTED", rejected, "EXPIRED", expired,
        "EXPIRED_IN_MATCH", expired_in_match);
};

template<>
struct glz::meta<binapi2::fapi::types::stp_mode>
{
    using enum binapi2::fapi::types::stp_mode;
    static constexpr auto value = enumerate("EXPIRE_TAKER", expire_taker, "EXPIRE_BOTH", expire_both, "EXPIRE_MAKER", expire_maker);
};

template<>
struct glz::meta<binapi2::fapi::types::price_match>
{
    using enum binapi2::fapi::types::price_match;
    static constexpr auto value = enumerate(
        "NONE", none, "OPPONENT", opponent, "OPPONENT_5", opponent_5, "OPPONENT_10", opponent_10,
        "OPPONENT_20", opponent_20, "QUEUE", queue, "QUEUE_5", queue_5, "QUEUE_10", queue_10,
        "QUEUE_20", queue_20);
};

template<>
struct glz::meta<binapi2::fapi::types::income_type>
{
    using enum binapi2::fapi::types::income_type;
    static constexpr auto value = enumerate(
        "TRANSFER", transfer, "WELCOME_BONUS", welcome_bonus, "REALIZED_PNL", realized_pnl,
        "FUNDING_FEE", funding_fee, "COMMISSION", commission, "INSURANCE_CLEAR", insurance_clear,
        "REFERRAL_KICKBACK", referral_kickback, "COMMISSION_REBATE", commission_rebate,
        "API_REBATE", api_rebate, "CONTEST_REWARD", contest_reward,
        "CROSS_COLLATERAL_TRANSFER", cross_collateral_transfer,
        "OPTIONS_PREMIUM_FEE", options_premium_fee, "OPTIONS_SETTLE_PROFIT", options_settle_profit,
        "INTERNAL_TRANSFER", internal_transfer, "AUTO_EXCHANGE", auto_exchange,
        "DELIVERED_SETTELMENT", delivered_settelment, "COIN_SWAP_DEPOSIT", coin_swap_deposit,
        "COIN_SWAP_WITHDRAW", coin_swap_withdraw, "POSITION_LIMIT_INCREASE_FEE", position_limit_increase_fee);
};

template<>
struct glz::meta<binapi2::fapi::types::futures_data_period>
{
    using enum binapi2::fapi::types::futures_data_period;
    static constexpr auto value = enumerate(
        "5m", m5, "15m", m15, "30m", m30, "1h", h1, "2h", h2, "4h", h4, "6h", h6, "12h", h12, "1d", d1);
};

template<>
struct glz::meta<binapi2::fapi::types::execution_type>
{
    using enum binapi2::fapi::types::execution_type;
    static constexpr auto value = enumerate(
        "NEW", new_order, "PARTIAL_FILL", partial_fill, "FILL", fill,
        "CANCELED", canceled, "REJECTED", rejected, "EXPIRED", expired, "TRADE", trade);
};

template<>
struct glz::meta<binapi2::fapi::types::rate_limit_type>
{
    using enum binapi2::fapi::types::rate_limit_type;
    static constexpr auto value = enumerate(
        "REQUEST_WEIGHT", request_weight, "ORDERS_1S", orders_1s, "ORDERS_1M", orders_1m,
        "ORDERS_1H", orders_1h, "ORDERS_1D", orders_1d);
};

template<>
struct glz::meta<binapi2::fapi::types::rate_limit_interval>
{
    using enum binapi2::fapi::types::rate_limit_interval;
    static constexpr auto value = enumerate("SECOND", second, "MINUTE", minute, "HOUR", hour, "DAY", day);
};

template<>
struct glz::meta<binapi2::fapi::types::auto_close_type>
{
    using enum binapi2::fapi::types::auto_close_type;
    static constexpr auto value = enumerate("LIQUIDATION", liquidation, "ADL", adl);
};

template<>
struct glz::meta<binapi2::fapi::types::delta_type>
{
    using enum binapi2::fapi::types::delta_type;
    static constexpr auto value = enumerate("1", add, "2", reduce);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_type>
{
    using enum binapi2::fapi::types::algo_type;
    static constexpr auto value = enumerate("TWAP", twap, "VP", vp);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_status>
{
    using enum binapi2::fapi::types::algo_status;
    static constexpr auto value = enumerate(
        "WORKING", working, "CANCELLED", cancelled, "REJECTED", rejected,
        "EXPIRED", expired, "TRIGGERED", triggered);
};
