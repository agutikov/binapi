// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file enums.hpp
/// @brief Enumeration types for the Binance USD-M Futures API.
///
/// Each enum class maps to a Binance API constant set. The corresponding
/// to_string() overload converts each enumerator to the exact API wire-format
/// string (e.g. order_side_t::buy -> "BUY").

#pragma once

#include <glaze/glaze.hpp>

#include <string>

namespace binapi2::fapi::types {

/// API endpoint security classification. Determines what credentials
/// are required to call an endpoint.
enum class security_type_t
{
    none,
    market_data,
    user_stream,
    user_data,
    trade,
};

/// Order side: buy or sell.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_side_t
{
    buy,
    sell,
};

/// Order type: LIMIT, MARKET, and conditional order variants (STOP, TAKE_PROFIT, etc.).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_type_t
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
enum class time_in_force_t
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
enum class kline_interval_t
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
enum class position_side_t
{
    both,
    long_side,   ///< Maps to "LONG" on the wire (avoids C++ keyword).
    short_side,  ///< Maps to "SHORT" on the wire (avoids C++ keyword).
};

/// Price type used for triggering conditional orders.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class working_type_t
{
    mark_price_t,
    contract_price,
};

/// Controls the detail level of order placement responses.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class response_type_t
{
    ack,
    result,
};

/// Position margin mode: isolated or cross (crossed).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class margin_type_t
{
    isolated,
    crossed,
};

/// Futures margining type (USD-M vs COIN-M).
enum class futures_type_t
{
    u_margined,
    coin_margined,
};

[[nodiscard]] inline std::string
to_string(futures_type_t value)
{
    switch (value) {
        case futures_type_t::u_margined:   return "U_MARGINED";
        case futures_type_t::coin_margined: return "COIN_MARGINED";
    }
    return "U_MARGINED";
}

/// Position control side for POSITION_RISK_CONTROL filters.
enum class position_control_side_t
{
    none,
    long_side,
    short_side,
    both,
};

[[nodiscard]] inline std::string
to_string(position_control_side_t value)
{
    switch (value) {
        case position_control_side_t::none:       return "NONE";
        case position_control_side_t::long_side:  return "LONG";
        case position_control_side_t::short_side: return "SHORT";
        case position_control_side_t::both:       return "BOTH";
    }
    return "NONE";
}

/// Trading permission type for symbol permissionSets.
enum class trading_permission_t
{
    grid,
    copy,
    dca,
    rpi,
    psb,
};

[[nodiscard]] inline std::string
to_string(trading_permission_t value)
{
    switch (value) {
        case trading_permission_t::grid: return "GRID";
        case trading_permission_t::copy: return "COPY";
        case trading_permission_t::dca:  return "DCA";
        case trading_permission_t::rpi:  return "RPI";
        case trading_permission_t::psb:  return "PSB";
    }
    return "GRID";
}

/// Futures contract delivery type (perpetual, quarterly, etc.).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class contract_type_t
{
    perpetual,
    current_month,
    next_month,
    current_quarter,
    next_quarter,
    perpetual_delivering,
    current_quarter_delivering,
    tradifi_perpetual,
};

/// Trading status of a futures contract through its lifecycle.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class contract_status_t
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
enum class order_status_t
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
enum class stp_mode_t
{
    none,
    expire_taker,
    expire_both,
    expire_maker,
};

/// Price match mode for order placement. Determines how the order price
/// is derived relative to the current order book (opponent/queue side,
/// with optional tick offset).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class price_match_t
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
enum class income_type_t
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
enum class futures_data_period_t
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
to_string(security_type_t value)
{
    switch (value) {
        case security_type_t::none:
            return "none";
        case security_type_t::market_data:
            return "market_data";
        case security_type_t::user_stream:
            return "user_stream";
        case security_type_t::user_data:
            return "user_data";
        case security_type_t::trade:
            return "trade";
    }
    return "none";
}

[[nodiscard]] inline std::string
to_string(order_side_t value)
{
    switch (value) {
        case order_side_t::buy:
            return "BUY";
        case order_side_t::sell:
            return "SELL";
    }
    return "BUY";
}

[[nodiscard]] inline std::string
to_string(order_type_t value)
{
    switch (value) {
        case order_type_t::limit:
            return "LIMIT";
        case order_type_t::market:
            return "MARKET";
        case order_type_t::stop:
            return "STOP";
        case order_type_t::stop_market:
            return "STOP_MARKET";
        case order_type_t::take_profit:
            return "TAKE_PROFIT";
        case order_type_t::take_profit_market:
            return "TAKE_PROFIT_MARKET";
        case order_type_t::trailing_stop_market:
            return "TRAILING_STOP_MARKET";
    }
    return "LIMIT";
}

[[nodiscard]] inline std::string
to_string(time_in_force_t value)
{
    switch (value) {
        case time_in_force_t::gtc:
            return "GTC";
        case time_in_force_t::ioc:
            return "IOC";
        case time_in_force_t::fok:
            return "FOK";
        case time_in_force_t::gtx:
            return "GTX";
        case time_in_force_t::gtd:
            return "GTD";
        case time_in_force_t::rpi:
            return "RPI";
    }
    return "GTC";
}

[[nodiscard]] inline std::string
to_string(kline_interval_t value)
{
    switch (value) {
        case kline_interval_t::m1:
            return "1m";
        case kline_interval_t::m3:
            return "3m";
        case kline_interval_t::m5:
            return "5m";
        case kline_interval_t::m15:
            return "15m";
        case kline_interval_t::m30:
            return "30m";
        case kline_interval_t::h1:
            return "1h";
        case kline_interval_t::h2:
            return "2h";
        case kline_interval_t::h4:
            return "4h";
        case kline_interval_t::h6:
            return "6h";
        case kline_interval_t::h8:
            return "8h";
        case kline_interval_t::h12:
            return "12h";
        case kline_interval_t::d1:
            return "1d";
        case kline_interval_t::d3:
            return "3d";
        case kline_interval_t::w1:
            return "1w";
        case kline_interval_t::mo1:
            return "1M";
    }
    return "1m";
}

[[nodiscard]] inline std::string
to_string(position_side_t value)
{
    switch (value) {
        case position_side_t::both:
            return "BOTH";
        case position_side_t::long_side:
            return "LONG";
        case position_side_t::short_side:
            return "SHORT";
    }
    return "BOTH";
}

[[nodiscard]] inline std::string
to_string(working_type_t value)
{
    switch (value) {
        case working_type_t::mark_price_t:
            return "MARK_PRICE";
        case working_type_t::contract_price:
            return "CONTRACT_PRICE";
    }
    return "MARK_PRICE";
}

[[nodiscard]] inline std::string
to_string(response_type_t value)
{
    switch (value) {
        case response_type_t::ack:
            return "ACK";
        case response_type_t::result:
            return "RESULT";
    }
    return "ACK";
}

[[nodiscard]] inline std::string
to_string(margin_type_t value)
{
    switch (value) {
        case margin_type_t::isolated:
            return "ISOLATED";
        case margin_type_t::crossed:
            return "CROSSED";
    }
    return "ISOLATED";
}

[[nodiscard]] inline std::string
to_string(contract_type_t value)
{
    switch (value) {
        case contract_type_t::perpetual:
            return "PERPETUAL";
        case contract_type_t::current_month:
            return "CURRENT_MONTH";
        case contract_type_t::next_month:
            return "NEXT_MONTH";
        case contract_type_t::current_quarter:
            return "CURRENT_QUARTER";
        case contract_type_t::next_quarter:
            return "NEXT_QUARTER";
        case contract_type_t::perpetual_delivering:
            return "PERPETUAL_DELIVERING";
        case contract_type_t::current_quarter_delivering:
            return "CURRENT_QUARTER DELIVERING";
        case contract_type_t::tradifi_perpetual:
            return "TRADIFI_PERPETUAL";
    }
    return "PERPETUAL";
}

[[nodiscard]] inline std::string
to_string(contract_status_t value)
{
    switch (value) {
        case contract_status_t::pending_trading:
            return "PENDING_TRADING";
        case contract_status_t::trading:
            return "TRADING";
        case contract_status_t::pre_delivering:
            return "PRE_DELIVERING";
        case contract_status_t::delivering:
            return "DELIVERING";
        case contract_status_t::delivered:
            return "DELIVERED";
        case contract_status_t::pre_settle:
            return "PRE_SETTLE";
        case contract_status_t::settling:
            return "SETTLING";
        case contract_status_t::close:
            return "CLOSE";
    }
    return "TRADING";
}

[[nodiscard]] inline std::string
to_string(order_status_t value)
{
    switch (value) {
        case order_status_t::new_order:
            return "NEW";
        case order_status_t::partially_filled:
            return "PARTIALLY_FILLED";
        case order_status_t::filled:
            return "FILLED";
        case order_status_t::canceled:
            return "CANCELED";
        case order_status_t::rejected:
            return "REJECTED";
        case order_status_t::expired:
            return "EXPIRED";
        case order_status_t::expired_in_match:
            return "EXPIRED_IN_MATCH";
    }
    return "NEW";
}

[[nodiscard]] inline std::string
to_string(stp_mode_t value)
{
    switch (value) {
        case stp_mode_t::none:
            return "NONE";
        case stp_mode_t::expire_taker:
            return "EXPIRE_TAKER";
        case stp_mode_t::expire_both:
            return "EXPIRE_BOTH";
        case stp_mode_t::expire_maker:
            return "EXPIRE_MAKER";
    }
    return "NONE";
}

[[nodiscard]] inline std::string
to_string(price_match_t value)
{
    switch (value) {
        case price_match_t::none:
            return "NONE";
        case price_match_t::opponent:
            return "OPPONENT";
        case price_match_t::opponent_5:
            return "OPPONENT_5";
        case price_match_t::opponent_10:
            return "OPPONENT_10";
        case price_match_t::opponent_20:
            return "OPPONENT_20";
        case price_match_t::queue:
            return "QUEUE";
        case price_match_t::queue_5:
            return "QUEUE_5";
        case price_match_t::queue_10:
            return "QUEUE_10";
        case price_match_t::queue_20:
            return "QUEUE_20";
    }
    return "NONE";
}

[[nodiscard]] inline std::string
to_string(income_type_t value)
{
    switch (value) {
        case income_type_t::transfer:
            return "TRANSFER";
        case income_type_t::welcome_bonus:
            return "WELCOME_BONUS";
        case income_type_t::realized_pnl:
            return "REALIZED_PNL";
        case income_type_t::funding_fee:
            return "FUNDING_FEE";
        case income_type_t::commission:
            return "COMMISSION";
        case income_type_t::insurance_clear:
            return "INSURANCE_CLEAR";
        case income_type_t::referral_kickback:
            return "REFERRAL_KICKBACK";
        case income_type_t::commission_rebate:
            return "COMMISSION_REBATE";
        case income_type_t::api_rebate:
            return "API_REBATE";
        case income_type_t::contest_reward:
            return "CONTEST_REWARD";
        case income_type_t::cross_collateral_transfer:
            return "CROSS_COLLATERAL_TRANSFER";
        case income_type_t::options_premium_fee:
            return "OPTIONS_PREMIUM_FEE";
        case income_type_t::options_settle_profit:
            return "OPTIONS_SETTLE_PROFIT";
        case income_type_t::internal_transfer:
            return "INTERNAL_TRANSFER";
        case income_type_t::auto_exchange:
            return "AUTO_EXCHANGE";
        case income_type_t::delivered_settelment:
            return "DELIVERED_SETTELMENT";
        case income_type_t::coin_swap_deposit:
            return "COIN_SWAP_DEPOSIT";
        case income_type_t::coin_swap_withdraw:
            return "COIN_SWAP_WITHDRAW";
        case income_type_t::position_limit_increase_fee:
            return "POSITION_LIMIT_INCREASE_FEE";
    }
    return "TRANSFER";
}

[[nodiscard]] inline std::string
to_string(futures_data_period_t value)
{
    switch (value) {
        case futures_data_period_t::m5:
            return "5m";
        case futures_data_period_t::m15:
            return "15m";
        case futures_data_period_t::m30:
            return "30m";
        case futures_data_period_t::h1:
            return "1h";
        case futures_data_period_t::h2:
            return "2h";
        case futures_data_period_t::h4:
            return "4h";
        case futures_data_period_t::h6:
            return "6h";
        case futures_data_period_t::h12:
            return "12h";
        case futures_data_period_t::d1:
            return "1d";
    }
    return "5m";
}

/// Execution type in order update events.
enum class execution_type_t
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
to_string(execution_type_t value)
{
    switch (value) {
        case execution_type_t::new_order: return "NEW";
        case execution_type_t::partial_fill: return "PARTIAL_FILL";
        case execution_type_t::fill: return "FILL";
        case execution_type_t::canceled: return "CANCELED";
        case execution_type_t::rejected: return "REJECTED";
        case execution_type_t::expired: return "EXPIRED";
        case execution_type_t::trade: return "TRADE";
    }
    return "NEW";
}

/// Rate limit type from exchange info.
enum class rate_limit_type_t
{
    request_weight,
    orders,
    orders_1s,
    orders_1m,
    orders_1h,
    orders_1d,
};

[[nodiscard]] inline std::string
to_string(rate_limit_type_t value)
{
    switch (value) {
        case rate_limit_type_t::request_weight: return "REQUEST_WEIGHT";
        case rate_limit_type_t::orders: return "ORDERS";
        case rate_limit_type_t::orders_1s: return "ORDERS_1S";
        case rate_limit_type_t::orders_1m: return "ORDERS_1M";
        case rate_limit_type_t::orders_1h: return "ORDERS_1H";
        case rate_limit_type_t::orders_1d: return "ORDERS_1D";
    }
    return "REQUEST_WEIGHT";
}

/// Rate limit interval unit.
enum class rate_limit_interval_t
{
    second,
    minute,
    hour,
    day,
};

[[nodiscard]] inline std::string
to_string(rate_limit_interval_t value)
{
    switch (value) {
        case rate_limit_interval_t::second: return "SECOND";
        case rate_limit_interval_t::minute: return "MINUTE";
        case rate_limit_interval_t::hour: return "HOUR";
        case rate_limit_interval_t::day: return "DAY";
    }
    return "SECOND";
}

/// Auto-close type for forced liquidation/ADL orders.
enum class auto_close_type_t
{
    liquidation,
    adl,
};

[[nodiscard]] inline std::string
to_string(auto_close_type_t value)
{
    switch (value) {
        case auto_close_type_t::liquidation: return "LIQUIDATION";
        case auto_close_type_t::adl: return "ADL";
    }
    return "LIQUIDATION";
}

/// Isolated margin delta direction.
enum class delta_type_t
{
    add,
    reduce,
};

[[nodiscard]] inline std::string
to_string(delta_type_t value)
{
    switch (value) {
        case delta_type_t::add: return "1";
        case delta_type_t::reduce: return "2";
    }
    return "1";
}

/// Algo order type.
enum class algo_type_t
{
    twap,
    vp,
};

[[nodiscard]] inline std::string
to_string(algo_type_t value)
{
    switch (value) {
        case algo_type_t::twap: return "TWAP";
        case algo_type_t::vp: return "VP";
    }
    return "TWAP";
}

/// Algo order status.
enum class algo_status_t
{
    working,
    cancelled,
    rejected,
    expired,
    triggered,
};

[[nodiscard]] inline std::string
to_string(algo_status_t value)
{
    switch (value) {
        case algo_status_t::working: return "WORKING";
        case algo_status_t::cancelled: return "CANCELLED";
        case algo_status_t::rejected: return "REJECTED";
        case algo_status_t::expired: return "EXPIRED";
        case algo_status_t::triggered: return "TRIGGERED";
    }
    return "WORKING";
}

} // namespace binapi2::fapi::types

// ---------------------------------------------------------------------------
// Glaze enum metadata — maps wire-format strings to enum values for JSON
// deserialization. Each specialization uses glz::enumerate("WIRE_NAME", value).
// ---------------------------------------------------------------------------

template<>
struct glz::meta<binapi2::fapi::types::order_side_t>
{
    using enum binapi2::fapi::types::order_side_t;
    static constexpr auto value = enumerate("BUY", buy, "SELL", sell);
};

template<>
struct glz::meta<binapi2::fapi::types::order_type_t>
{
    using enum binapi2::fapi::types::order_type_t;
    static constexpr auto value = enumerate(
        "LIMIT", limit, "MARKET", market, "STOP", stop, "STOP_MARKET", stop_market,
        "TAKE_PROFIT", take_profit, "TAKE_PROFIT_MARKET", take_profit_market,
        "TRAILING_STOP_MARKET", trailing_stop_market);
};

template<>
struct glz::meta<binapi2::fapi::types::time_in_force_t>
{
    using enum binapi2::fapi::types::time_in_force_t;
    static constexpr auto value = enumerate("GTC", gtc, "IOC", ioc, "FOK", fok, "GTX", gtx, "GTD", gtd, "RPI", rpi);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_interval_t>
{
    using enum binapi2::fapi::types::kline_interval_t;
    static constexpr auto value = enumerate(
        "1m", m1, "3m", m3, "5m", m5, "15m", m15, "30m", m30,
        "1h", h1, "2h", h2, "4h", h4, "6h", h6, "8h", h8, "12h", h12,
        "1d", d1, "3d", d3, "1w", w1, "1M", mo1);
};

template<>
struct glz::meta<binapi2::fapi::types::position_side_t>
{
    using enum binapi2::fapi::types::position_side_t;
    static constexpr auto value = enumerate("BOTH", both, "LONG", long_side, "SHORT", short_side);
};

template<>
struct glz::meta<binapi2::fapi::types::working_type_t>
{
    using enum binapi2::fapi::types::working_type_t;
    static constexpr auto value = enumerate("MARK_PRICE", mark_price_t, "CONTRACT_PRICE", contract_price);
};

template<>
struct glz::meta<binapi2::fapi::types::response_type_t>
{
    using enum binapi2::fapi::types::response_type_t;
    static constexpr auto value = enumerate("ACK", ack, "RESULT", result);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_type_t>
{
    using enum binapi2::fapi::types::margin_type_t;
    static constexpr auto value = enumerate("ISOLATED", isolated, "CROSSED", crossed, "isolated", isolated, "cross", crossed);
};

template<>
struct glz::meta<binapi2::fapi::types::futures_type_t>
{
    using enum binapi2::fapi::types::futures_type_t;
    static constexpr auto value = enumerate("U_MARGINED", u_margined, "COIN_MARGINED", coin_margined);
};

template<>
struct glz::meta<binapi2::fapi::types::position_control_side_t>
{
    using enum binapi2::fapi::types::position_control_side_t;
    static constexpr auto value = enumerate("NONE", none, "LONG", long_side, "SHORT", short_side, "BOTH", both);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_permission_t>
{
    using enum binapi2::fapi::types::trading_permission_t;
    static constexpr auto value = enumerate("GRID", grid, "COPY", copy, "DCA", dca, "RPI", rpi, "PSB", psb);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_type_t>
{
    using enum binapi2::fapi::types::contract_type_t;
    static constexpr auto value = enumerate(
        "PERPETUAL", perpetual, "CURRENT_MONTH", current_month, "NEXT_MONTH", next_month,
        "CURRENT_QUARTER", current_quarter, "NEXT_QUARTER", next_quarter,
        "PERPETUAL_DELIVERING", perpetual_delivering,
        "CURRENT_QUARTER DELIVERING", current_quarter_delivering,
        "TRADIFI_PERPETUAL", tradifi_perpetual);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_status_t>
{
    using enum binapi2::fapi::types::contract_status_t;
    static constexpr auto value = enumerate(
        "PENDING_TRADING", pending_trading, "TRADING", trading, "PRE_DELIVERING", pre_delivering,
        "DELIVERING", delivering, "DELIVERED", delivered, "PRE_SETTLE", pre_settle,
        "SETTLING", settling, "CLOSE", close);
};

template<>
struct glz::meta<binapi2::fapi::types::order_status_t>
{
    using enum binapi2::fapi::types::order_status_t;
    static constexpr auto value = enumerate(
        "NEW", new_order, "PARTIALLY_FILLED", partially_filled, "FILLED", filled,
        "CANCELED", canceled, "REJECTED", rejected, "EXPIRED", expired,
        "EXPIRED_IN_MATCH", expired_in_match);
};

template<>
struct glz::meta<binapi2::fapi::types::stp_mode_t>
{
    using enum binapi2::fapi::types::stp_mode_t;
    static constexpr auto value = enumerate("NONE", none, "EXPIRE_TAKER", expire_taker, "EXPIRE_BOTH", expire_both, "EXPIRE_MAKER", expire_maker);
};

template<>
struct glz::meta<binapi2::fapi::types::price_match_t>
{
    using enum binapi2::fapi::types::price_match_t;
    static constexpr auto value = enumerate(
        "NONE", none, "OPPONENT", opponent, "OPPONENT_5", opponent_5, "OPPONENT_10", opponent_10,
        "OPPONENT_20", opponent_20, "QUEUE", queue, "QUEUE_5", queue_5, "QUEUE_10", queue_10,
        "QUEUE_20", queue_20);
};

template<>
struct glz::meta<binapi2::fapi::types::income_type_t>
{
    using enum binapi2::fapi::types::income_type_t;
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
struct glz::meta<binapi2::fapi::types::futures_data_period_t>
{
    using enum binapi2::fapi::types::futures_data_period_t;
    static constexpr auto value = enumerate(
        "5m", m5, "15m", m15, "30m", m30, "1h", h1, "2h", h2, "4h", h4, "6h", h6, "12h", h12, "1d", d1);
};

template<>
struct glz::meta<binapi2::fapi::types::execution_type_t>
{
    using enum binapi2::fapi::types::execution_type_t;
    static constexpr auto value = enumerate(
        "NEW", new_order, "PARTIAL_FILL", partial_fill, "FILL", fill,
        "CANCELED", canceled, "REJECTED", rejected, "EXPIRED", expired, "TRADE", trade);
};

template<>
struct glz::meta<binapi2::fapi::types::rate_limit_type_t>
{
    using enum binapi2::fapi::types::rate_limit_type_t;
    static constexpr auto value = enumerate(
        "REQUEST_WEIGHT", request_weight, "ORDERS", orders,
        "ORDERS_1S", orders_1s, "ORDERS_1M", orders_1m,
        "ORDERS_1H", orders_1h, "ORDERS_1D", orders_1d);
};

template<>
struct glz::meta<binapi2::fapi::types::rate_limit_interval_t>
{
    using enum binapi2::fapi::types::rate_limit_interval_t;
    static constexpr auto value = enumerate("SECOND", second, "MINUTE", minute, "HOUR", hour, "DAY", day);
};

template<>
struct glz::meta<binapi2::fapi::types::auto_close_type_t>
{
    using enum binapi2::fapi::types::auto_close_type_t;
    static constexpr auto value = enumerate("LIQUIDATION", liquidation, "ADL", adl);
};

template<>
struct glz::meta<binapi2::fapi::types::delta_type_t>
{
    using enum binapi2::fapi::types::delta_type_t;
    static constexpr auto value = enumerate("1", add, "2", reduce);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_type_t>
{
    using enum binapi2::fapi::types::algo_type_t;
    static constexpr auto value = enumerate("TWAP", twap, "VP", vp);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_status_t>
{
    using enum binapi2::fapi::types::algo_status_t;
    static constexpr auto value = enumerate(
        "WORKING", working, "CANCELLED", cancelled, "REJECTED", rejected,
        "EXPIRED", expired, "TRIGGERED", triggered);
};
