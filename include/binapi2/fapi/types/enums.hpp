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

#include <stdexcept>
#include <string>

namespace binapi2::fapi::types {

/// API endpoint security classification. Determines what credentials
/// are required to call an endpoint.
enum class security_type_t
{
    none = 0,
    market_data = 1,
    user_stream = 2,
    user_data = 3,
    trade = 4,
};

/// Order side: buy or sell.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_side_t
{
    buy = 0,
    sell = 1,
};

/// Order type: LIMIT, MARKET, and conditional order variants (STOP, TAKE_PROFIT, etc.).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_type_t
{
    limit = 0,
    market = 1,
    stop = 2,              ///< Stop-limit order (requires price + stopPrice).
    stop_market = 3,       ///< Stop-market order (requires stopPrice only).
    take_profit = 4,       ///< Take-profit limit order.
    take_profit_market = 5,///< Take-profit market order.
    trailing_stop_market = 6,
};

/// Time-in-force policy for order lifetime.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class time_in_force_t
{
    gtc = 0,  ///< Good Till Cancel.
    ioc = 1,  ///< Immediate Or Cancel.
    fok = 2,  ///< Fill Or Kill.
    gtx = 3,  ///< Good Till Crossing (post-only).
    gtd = 4,  ///< Good Till Date.
    rpi = 5,  ///< Retail Price Improvement.
};

/// Kline/candlestick interval. Prefixed by unit: m=minutes, h=hours, d=days, w=weeks, mo=months.
/// to_string() produces the API format (e.g. m1 -> "1m", mo1 -> "1M").
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class kline_interval_t
{
    m1 = 0,
    m3 = 1,
    m5 = 2,
    m15 = 3,
    m30 = 4,
    h1 = 5,
    h2 = 6,
    h4 = 7,
    h6 = 8,
    h8 = 9,
    h12 = 10,
    d1 = 11,
    d3 = 12,
    w1 = 13,
    mo1 = 14,
};

/// Position side for hedge mode. "both" is used in one-way mode.
/// long_side/short_side map to API strings "LONG"/"SHORT".
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class position_side_t
{
    both = 0,
    long_side = 1,   ///< Maps to "LONG" on the wire (avoids C++ keyword).
    short_side = 2,  ///< Maps to "SHORT" on the wire (avoids C++ keyword).
};

/// Price type used for triggering conditional orders.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class working_type_t
{
    mark_price_t = 0,
    contract_price = 1,
};

/// Controls the detail level of order placement responses.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class response_type_t
{
    ack = 0,
    result = 1,
};

/// Position margin mode: isolated or cross (crossed).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class margin_type_t
{
    isolated = 0,
    crossed = 1,
};

/// Futures margining type (USD-M vs COIN-M).
enum class futures_type_t
{
    u_margined = 0,
    coin_margined = 1,
};

[[nodiscard]] inline std::string
to_string(futures_type_t value)
{
    switch (value) {
        case futures_type_t::u_margined:   return "U_MARGINED";
        case futures_type_t::coin_margined: return "COIN_MARGINED";
    }
    throw std::invalid_argument("invalid futures_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Position control side for POSITION_RISK_CONTROL filters.
enum class position_control_side_t
{
    none = 0,
    long_side = 1,
    short_side = 2,
    both = 3,
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
    throw std::invalid_argument("invalid position_control_side_t: " + std::to_string(static_cast<int>(value)));
}

/// Trading permission type for symbol permissionSets.
enum class trading_permission_t
{
    grid = 0,
    copy = 1,
    dca = 2,
    rpi = 3,
    psb = 4,
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
    throw std::invalid_argument("invalid trading_permission_t: " + std::to_string(static_cast<int>(value)));
}

/// Futures contract delivery type (perpetual, quarterly, etc.).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class contract_type_t
{
    perpetual = 0,
    current_month = 1,
    next_month = 2,
    current_quarter = 3,
    next_quarter = 4,
    perpetual_delivering = 5,
    current_quarter_delivering = 6,
    tradifi_perpetual = 7,
};

/// Trading status of a futures contract through its lifecycle.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class contract_status_t
{
    pending_trading = 0,
    trading = 1,
    pre_delivering = 2,
    delivering = 3,
    delivered = 4,
    pre_settle = 5,
    settling = 6,
    close = 7,
};

/// Order execution status. Note: new_order maps to API string "NEW"
/// (renamed to avoid collision with C++ keywords/macros).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class order_status_t
{
    new_order = 0,
    partially_filled = 1,
    filled = 2,
    canceled = 3,
    rejected = 4,
    expired = 5,
    expired_in_match = 6,
};

/// Self-Trade Prevention mode. Controls which side is expired when
/// an order would match against another order from the same account.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class stp_mode_t
{
    none = 0,
    expire_taker = 1,
    expire_both = 2,
    expire_maker = 3,
};

/// Price match mode for order placement. Determines how the order price
/// is derived relative to the current order book (opponent/queue side,
/// with optional tick offset).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class price_match_t
{
    none = 0,
    opponent = 1,
    opponent_5 = 2,
    opponent_10 = 3,
    opponent_20 = 4,
    queue = 5,
    queue_5 = 6,
    queue_10 = 7,
    queue_20 = 8,
};

/// Classification of account income/transaction entries returned by
/// the income history endpoint.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class income_type_t
{
    transfer = 0,
    welcome_bonus = 1,
    realized_pnl = 2,
    funding_fee = 3,
    commission = 4,
    insurance_clear = 5,
    referral_kickback = 6,
    commission_rebate = 7,
    api_rebate = 8,
    contest_reward = 9,
    cross_collateral_transfer = 10,
    options_premium_fee = 11,
    options_settle_profit = 12,
    internal_transfer = 13,
    auto_exchange = 14,
    delivered_settelment = 15,
    coin_swap_deposit = 16,
    coin_swap_withdraw = 17,
    position_limit_increase_fee = 18,
};

/// Aggregation period for futures statistics endpoints (open interest stats,
/// long/short ratio, taker volume, basis). to_string() produces API format
/// (e.g. m5 -> "5m").
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
enum class futures_data_period_t
{
    m5 = 0,
    m15 = 1,
    m30 = 2,
    h1 = 3,
    h2 = 4,
    h4 = 5,
    h6 = 6,
    h12 = 7,
    d1 = 8,
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
    throw std::invalid_argument("invalid security_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid order_side_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid order_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid time_in_force_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid kline_interval_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid position_side_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid working_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid response_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid margin_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid contract_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid contract_status_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid order_status_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid stp_mode_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid price_match_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid income_type_t: " + std::to_string(static_cast<int>(value)));
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
    throw std::invalid_argument("invalid futures_data_period_t: " + std::to_string(static_cast<int>(value)));
}

/// Execution type in order update events.
enum class execution_type_t
{
    new_order = 0,
    partial_fill = 1,
    fill = 2,
    canceled = 3,
    rejected = 4,
    expired = 5,
    trade = 6,
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
    throw std::invalid_argument("invalid execution_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Rate limit type from exchange info.
enum class rate_limit_type_t
{
    request_weight = 0,
    orders = 1,
    orders_1s = 2,
    orders_1m = 3,
    orders_1h = 4,
    orders_1d = 5,
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
    throw std::invalid_argument("invalid rate_limit_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Rate limit interval unit.
enum class rate_limit_interval_t
{
    second = 0,
    minute = 1,
    hour = 2,
    day = 3,
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
    throw std::invalid_argument("invalid rate_limit_interval_t: " + std::to_string(static_cast<int>(value)));
}

/// Auto-close type for forced liquidation/ADL orders.
enum class auto_close_type_t
{
    liquidation = 0,
    adl = 1,
};

[[nodiscard]] inline std::string
to_string(auto_close_type_t value)
{
    switch (value) {
        case auto_close_type_t::liquidation: return "LIQUIDATION";
        case auto_close_type_t::adl: return "ADL";
    }
    throw std::invalid_argument("invalid auto_close_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Isolated margin delta direction.
enum class delta_type_t
{
    add = 1,
    reduce = 2,
};

[[nodiscard]] inline std::string
to_string(delta_type_t value)
{
    switch (value) {
        case delta_type_t::add: return "1";
        case delta_type_t::reduce: return "2";
    }
    throw std::invalid_argument("invalid delta_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Market stream event type (field "e" in WebSocket market data events).
enum class market_event_type_t
{
    agg_trade = 0,
    mark_price_update = 1,
    depth_update = 2,
    mini_ticker_24hr = 3,
    ticker_24hr = 4,
    force_order = 5,
    kline = 6,
    continuous_kline = 7,
    composite_index = 8,
    contract_info = 9,
    asset_index_update = 10,
    book_ticker = 11,
    equity_update = 12,
    commodity_update = 13,
};

[[nodiscard]] inline std::string
to_string(market_event_type_t value)
{
    switch (value) {
        case market_event_type_t::agg_trade: return "aggTrade";
        case market_event_type_t::mark_price_update: return "markPriceUpdate";
        case market_event_type_t::depth_update: return "depthUpdate";
        case market_event_type_t::mini_ticker_24hr: return "24hrMiniTicker";
        case market_event_type_t::ticker_24hr: return "24hrTicker";
        case market_event_type_t::force_order: return "forceOrder";
        case market_event_type_t::kline: return "kline";
        case market_event_type_t::continuous_kline: return "continuous_kline";
        case market_event_type_t::composite_index: return "compositeIndex";
        case market_event_type_t::contract_info: return "contractInfo";
        case market_event_type_t::asset_index_update: return "assetIndexUpdate";
        case market_event_type_t::book_ticker: return "bookTicker";
        case market_event_type_t::equity_update: return "EquityUpdate";
        case market_event_type_t::commodity_update: return "CommodityUpdate";
    }
    throw std::invalid_argument("invalid market_event_type_t: " + std::to_string(static_cast<int>(value)));
}

/// User data stream event type (field "e" in WebSocket user data events).
enum class user_event_type_t
{
    account_update = 0,
    order_trade_update = 1,
    margin_call = 2,
    listen_key_expired = 3,
    account_config_update = 4,
    trade_lite = 5,
    algo_update = 6,
    conditional_order_trigger_reject = 7,
    grid_update = 8,
    strategy_update = 9,
};

[[nodiscard]] inline std::string
to_string(user_event_type_t value)
{
    switch (value) {
        case user_event_type_t::account_update: return "ACCOUNT_UPDATE";
        case user_event_type_t::order_trade_update: return "ORDER_TRADE_UPDATE";
        case user_event_type_t::margin_call: return "MARGIN_CALL";
        case user_event_type_t::listen_key_expired: return "listenKeyExpired";
        case user_event_type_t::account_config_update: return "ACCOUNT_CONFIG_UPDATE";
        case user_event_type_t::trade_lite: return "TRADE_LITE";
        case user_event_type_t::algo_update: return "ALGO_UPDATE";
        case user_event_type_t::conditional_order_trigger_reject: return "CONDITIONAL_ORDER_TRIGGER_REJECT";
        case user_event_type_t::grid_update: return "GRID_UPDATE";
        case user_event_type_t::strategy_update: return "STRATEGY_UPDATE";
    }
    throw std::invalid_argument("invalid user_event_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Algo order type.
enum class algo_type_t
{
    twap = 0,
    vp = 1,
};

[[nodiscard]] inline std::string
to_string(algo_type_t value)
{
    switch (value) {
        case algo_type_t::twap: return "TWAP";
        case algo_type_t::vp: return "VP";
    }
    throw std::invalid_argument("invalid algo_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Algo order status.
enum class algo_status_t
{
    working = 0,
    cancelled = 1,
    rejected = 2,
    expired = 3,
    triggered = 4,
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
    throw std::invalid_argument("invalid algo_status_t: " + std::to_string(static_cast<int>(value)));
}

/// Account update reason type (field "m" in ACCOUNT_UPDATE events).
enum class reason_type_t
{
    deposit = 0,
    withdraw = 1,
    order = 2,
    funding_fee = 3,
    withdraw_reject = 4,
    adjustment = 5,
    insurance_clear = 6,
    admin_deposit = 7,
    admin_withdraw = 8,
    margin_transfer = 9,
    margin_type_change = 10,
    asset_transfer = 11,
    options_premium_fee = 12,
    options_settle_profit = 13,
    auto_exchange = 14,
    coin_swap_deposit = 15,
    coin_swap_withdraw = 16,
};

[[nodiscard]] inline std::string
to_string(reason_type_t value)
{
    switch (value) {
        case reason_type_t::deposit: return "DEPOSIT";
        case reason_type_t::withdraw: return "WITHDRAW";
        case reason_type_t::order: return "ORDER";
        case reason_type_t::funding_fee: return "FUNDING_FEE";
        case reason_type_t::withdraw_reject: return "WITHDRAW_REJECT";
        case reason_type_t::adjustment: return "ADJUSTMENT";
        case reason_type_t::insurance_clear: return "INSURANCE_CLEAR";
        case reason_type_t::admin_deposit: return "ADMIN_DEPOSIT";
        case reason_type_t::admin_withdraw: return "ADMIN_WITHDRAW";
        case reason_type_t::margin_transfer: return "MARGIN_TRANSFER";
        case reason_type_t::margin_type_change: return "MARGIN_TYPE_CHANGE";
        case reason_type_t::asset_transfer: return "ASSET_TRANSFER";
        case reason_type_t::options_premium_fee: return "OPTIONS_PREMIUM_FEE";
        case reason_type_t::options_settle_profit: return "OPTIONS_SETTLE_PROFIT";
        case reason_type_t::auto_exchange: return "AUTO_EXCHANGE";
        case reason_type_t::coin_swap_deposit: return "COIN_SWAP_DEPOSIT";
        case reason_type_t::coin_swap_withdraw: return "COIN_SWAP_WITHDRAW";
    }
    throw std::invalid_argument("invalid reason_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Trading session type (field "S" in trading session stream events).
enum class session_type_t
{
    pre_market = 0,
    regular = 1,
    after_market = 2,
    overnight = 3,
    no_trading = 4,
};

[[nodiscard]] inline std::string
to_string(session_type_t value)
{
    switch (value) {
        case session_type_t::pre_market: return "PRE_MARKET";
        case session_type_t::regular: return "REGULAR";
        case session_type_t::after_market: return "AFTER_MARKET";
        case session_type_t::overnight: return "OVERNIGHT";
        case session_type_t::no_trading: return "NO_TRADING";
    }
    throw std::invalid_argument("invalid session_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Grid/strategy type (field "st" in GRID_UPDATE / STRATEGY_UPDATE events).
enum class strategy_type_t
{
    grid = 0,
};

[[nodiscard]] inline std::string
to_string(strategy_type_t value)
{
    switch (value) {
        case strategy_type_t::grid: return "GRID";
    }
    throw std::invalid_argument("invalid strategy_type_t: " + std::to_string(static_cast<int>(value)));
}

/// Grid/strategy status (field "ss" in GRID_UPDATE / STRATEGY_UPDATE events).
enum class strategy_status_t
{
    new_strategy = 0,
    working = 1,
    cancelled = 2,
    expired = 3,
};

[[nodiscard]] inline std::string
to_string(strategy_status_t value)
{
    switch (value) {
        case strategy_status_t::new_strategy: return "NEW";
        case strategy_status_t::working: return "WORKING";
        case strategy_status_t::cancelled: return "CANCELLED";
        case strategy_status_t::expired: return "EXPIRED";
    }
    throw std::invalid_argument("invalid strategy_status_t: " + std::to_string(static_cast<int>(value)));
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

template<>
struct glz::meta<binapi2::fapi::types::market_event_type_t>
{
    using enum binapi2::fapi::types::market_event_type_t;
    static constexpr auto value = enumerate(
        "aggTrade", agg_trade, "markPriceUpdate", mark_price_update,
        "depthUpdate", depth_update, "24hrMiniTicker", mini_ticker_24hr,
        "24hrTicker", ticker_24hr, "forceOrder", force_order,
        "kline", kline, "continuous_kline", continuous_kline,
        "compositeIndex", composite_index, "contractInfo", contract_info,
        "assetIndexUpdate", asset_index_update, "bookTicker", book_ticker,
        "EquityUpdate", equity_update, "CommodityUpdate", commodity_update);
};

template<>
struct glz::meta<binapi2::fapi::types::user_event_type_t>
{
    using enum binapi2::fapi::types::user_event_type_t;
    static constexpr auto value = enumerate(
        "ACCOUNT_UPDATE", account_update, "ORDER_TRADE_UPDATE", order_trade_update,
        "MARGIN_CALL", margin_call, "listenKeyExpired", listen_key_expired,
        "ACCOUNT_CONFIG_UPDATE", account_config_update, "TRADE_LITE", trade_lite,
        "ALGO_UPDATE", algo_update, "CONDITIONAL_ORDER_TRIGGER_REJECT", conditional_order_trigger_reject,
        "GRID_UPDATE", grid_update, "STRATEGY_UPDATE", strategy_update);
};

template<>
struct glz::meta<binapi2::fapi::types::reason_type_t>
{
    using enum binapi2::fapi::types::reason_type_t;
    static constexpr auto value = enumerate(
        "DEPOSIT", deposit, "WITHDRAW", withdraw, "ORDER", order,
        "FUNDING_FEE", funding_fee, "WITHDRAW_REJECT", withdraw_reject,
        "ADJUSTMENT", adjustment, "INSURANCE_CLEAR", insurance_clear,
        "ADMIN_DEPOSIT", admin_deposit, "ADMIN_WITHDRAW", admin_withdraw,
        "MARGIN_TRANSFER", margin_transfer, "MARGIN_TYPE_CHANGE", margin_type_change,
        "ASSET_TRANSFER", asset_transfer, "OPTIONS_PREMIUM_FEE", options_premium_fee,
        "OPTIONS_SETTLE_PROFIT", options_settle_profit, "AUTO_EXCHANGE", auto_exchange,
        "COIN_SWAP_DEPOSIT", coin_swap_deposit, "COIN_SWAP_WITHDRAW", coin_swap_withdraw);
};

template<>
struct glz::meta<binapi2::fapi::types::session_type_t>
{
    using enum binapi2::fapi::types::session_type_t;
    static constexpr auto value = enumerate(
        "PRE_MARKET", pre_market, "REGULAR", regular, "AFTER_MARKET", after_market,
        "OVERNIGHT", overnight, "NO_TRADING", no_trading);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_type_t>
{
    using enum binapi2::fapi::types::strategy_type_t;
    static constexpr auto value = enumerate("GRID", grid);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_status_t>
{
    using enum binapi2::fapi::types::strategy_status_t;
    static constexpr auto value = enumerate(
        "NEW", new_strategy, "WORKING", working, "CANCELLED", cancelled, "EXPIRED", expired);
};
