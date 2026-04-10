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
#include <string_view>
#include <type_traits>
#include <utility>

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

/// Position control side for POSITION_RISK_CONTROL filters.
enum class position_control_side_t
{
    none = 0,
    long_side = 1,
    short_side = 2,
    both = 3,
};

/// Trading permission type for symbol permissionSets.
enum class trading_permission_t
{
    grid = 0,
    copy = 1,
    dca = 2,
    rpi = 3,
    psb = 4,
};

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

/// Rate limit interval unit.
enum class rate_limit_interval_t
{
    second = 0,
    minute = 1,
    hour = 2,
    day = 3,
};

/// Auto-close type for forced liquidation/ADL orders.
enum class auto_close_type_t
{
    liquidation = 0,
    adl = 1,
};

/// Isolated margin delta direction.
enum class delta_type_t
{
    add = 1,
    reduce = 2,
};

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
    trading_session = 14,
};

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

/// Algo order type.
enum class algo_type_t
{
    twap = 0,
    vp = 1,
};

/// Algo order status.
enum class algo_status_t
{
    working = 0,
    cancelled = 1,
    rejected = 2,
    expired = 3,
    triggered = 4,
};

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

/// Trading session type (field "S" in trading session stream events).
enum class session_type_t
{
    pre_market = 0,
    regular = 1,
    after_market = 2,
    overnight = 3,
    no_trading = 4,
};

/// Grid/strategy type (field "st" in GRID_UPDATE / STRATEGY_UPDATE events).
enum class strategy_type_t
{
    grid = 0,
};

/// Grid/strategy status (field "ss" in GRID_UPDATE / STRATEGY_UPDATE events).
enum class strategy_status_t
{
    new_strategy = 0,
    working = 1,
    cancelled = 2,
    expired = 3,
};

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
        "EquityUpdate", equity_update, "CommodityUpdate", commodity_update,
        "tradingSession", trading_session);
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

template<>
struct glz::meta<binapi2::fapi::types::security_type_t>
{
    using enum binapi2::fapi::types::security_type_t;
    static constexpr auto value = enumerate(
        "none", none, "market_data", market_data, "user_stream", user_stream,
        "user_data", user_data, "trade", trade);
};

// ---------------------------------------------------------------------------
// Generic to_string for all enum types that have a glz::meta specialization.
// Walks the glaze enumerate tuple to find the wire-format string for a value.
// ---------------------------------------------------------------------------

namespace binapi2::fapi::types {

namespace detail {

template<class T, class Tuple, std::size_t... Is>
[[nodiscard]] constexpr std::string_view
enum_to_sv_impl(T value, const Tuple& tup, std::index_sequence<Is...>)
{
    std::string_view result{};
    (void)((glz::get<Is * 2 + 1>(tup) == value ? (result = glz::get<Is * 2>(tup), true) : false) || ...);
    return result;
}

} // namespace detail

template<class T>
    requires std::is_enum_v<T>
[[nodiscard]] inline std::string
to_string(T value)
{
    constexpr auto& tup = glz::meta<T>::value.value;
    constexpr auto N = glz::tuple_size_v<std::decay_t<decltype(tup)>> / 2;
    auto sv = detail::enum_to_sv_impl(value, tup, std::make_index_sequence<N>{});
    if (!sv.empty()) [[likely]]
        return std::string(sv);
    throw std::invalid_argument("invalid enum value: " + std::to_string(static_cast<int>(value)));
}

} // namespace binapi2::fapi::types
