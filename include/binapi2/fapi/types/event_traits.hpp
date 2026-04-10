// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file event_traits.hpp
/// @brief Compile-time mapping between stream event struct types, wire strings, and enum values.

#pragma once

#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/user_stream_events.hpp>

#include <string_view>

namespace binapi2::fapi::types {

template<typename Event>
struct event_traits;

// ---------------------------------------------------------------------------
// Market event traits
// ---------------------------------------------------------------------------

template<>
struct event_traits<book_ticker_stream_event_t>
{
    static constexpr std::string_view wire_name = "bookTicker";
    static constexpr market_event_type_t enum_value = market_event_type_t::book_ticker;
};

template<>
struct event_traits<aggregate_trade_stream_event_t>
{
    static constexpr std::string_view wire_name = "aggTrade";
    static constexpr market_event_type_t enum_value = market_event_type_t::agg_trade;
};

template<>
struct event_traits<mark_price_stream_event_t>
{
    static constexpr std::string_view wire_name = "markPriceUpdate";
    static constexpr market_event_type_t enum_value = market_event_type_t::mark_price_update;
};

template<>
struct event_traits<depth_stream_event_t>
{
    static constexpr std::string_view wire_name = "depthUpdate";
    static constexpr market_event_type_t enum_value = market_event_type_t::depth_update;
};

template<>
struct event_traits<mini_ticker_stream_event_t>
{
    static constexpr std::string_view wire_name = "24hrMiniTicker";
    static constexpr market_event_type_t enum_value = market_event_type_t::mini_ticker_24hr;
};

template<>
struct event_traits<ticker_stream_event_t>
{
    static constexpr std::string_view wire_name = "24hrTicker";
    static constexpr market_event_type_t enum_value = market_event_type_t::ticker_24hr;
};

template<>
struct event_traits<liquidation_order_stream_event_t>
{
    static constexpr std::string_view wire_name = "forceOrder";
    static constexpr market_event_type_t enum_value = market_event_type_t::force_order;
};

template<>
struct event_traits<kline_stream_event_t>
{
    static constexpr std::string_view wire_name = "kline";
    static constexpr market_event_type_t enum_value = market_event_type_t::kline;
};

template<>
struct event_traits<continuous_contract_kline_stream_event_t>
{
    static constexpr std::string_view wire_name = "continuous_kline";
    static constexpr market_event_type_t enum_value = market_event_type_t::continuous_kline;
};

template<>
struct event_traits<composite_index_stream_event_t>
{
    static constexpr std::string_view wire_name = "compositeIndex";
    static constexpr market_event_type_t enum_value = market_event_type_t::composite_index;
};

template<>
struct event_traits<contract_info_stream_event_t>
{
    static constexpr std::string_view wire_name = "contractInfo";
    static constexpr market_event_type_t enum_value = market_event_type_t::contract_info;
};

template<>
struct event_traits<asset_index_stream_event_t>
{
    static constexpr std::string_view wire_name = "assetIndexUpdate";
    static constexpr market_event_type_t enum_value = market_event_type_t::asset_index_update;
};

template<>
struct event_traits<trading_session_stream_event_t>
{
    static constexpr std::string_view wire_name = "tradingSession";
    static constexpr market_event_type_t enum_value = market_event_type_t::trading_session;
};

// ---------------------------------------------------------------------------
// User event traits
// ---------------------------------------------------------------------------

template<>
struct event_traits<account_update_event_t>
{
    static constexpr std::string_view wire_name = "ACCOUNT_UPDATE";
    static constexpr user_event_type_t enum_value = user_event_type_t::account_update;
};

template<>
struct event_traits<order_trade_update_event_t>
{
    static constexpr std::string_view wire_name = "ORDER_TRADE_UPDATE";
    static constexpr user_event_type_t enum_value = user_event_type_t::order_trade_update;
};

template<>
struct event_traits<margin_call_event_t>
{
    static constexpr std::string_view wire_name = "MARGIN_CALL";
    static constexpr user_event_type_t enum_value = user_event_type_t::margin_call;
};

template<>
struct event_traits<listen_key_expired_event_t>
{
    static constexpr std::string_view wire_name = "listenKeyExpired";
    static constexpr user_event_type_t enum_value = user_event_type_t::listen_key_expired;
};

template<>
struct event_traits<account_config_update_event_t>
{
    static constexpr std::string_view wire_name = "ACCOUNT_CONFIG_UPDATE";
    static constexpr user_event_type_t enum_value = user_event_type_t::account_config_update;
};

template<>
struct event_traits<trade_lite_event_t>
{
    static constexpr std::string_view wire_name = "TRADE_LITE";
    static constexpr user_event_type_t enum_value = user_event_type_t::trade_lite;
};

template<>
struct event_traits<algo_order_update_event_t>
{
    static constexpr std::string_view wire_name = "ALGO_UPDATE";
    static constexpr user_event_type_t enum_value = user_event_type_t::algo_update;
};

template<>
struct event_traits<conditional_order_trigger_reject_event_t>
{
    static constexpr std::string_view wire_name = "CONDITIONAL_ORDER_TRIGGER_REJECT";
    static constexpr user_event_type_t enum_value = user_event_type_t::conditional_order_trigger_reject;
};

template<>
struct event_traits<grid_update_event_t>
{
    static constexpr std::string_view wire_name = "GRID_UPDATE";
    static constexpr user_event_type_t enum_value = user_event_type_t::grid_update;
};

template<>
struct event_traits<strategy_update_event_t>
{
    static constexpr std::string_view wire_name = "STRATEGY_UPDATE";
    static constexpr user_event_type_t enum_value = user_event_type_t::strategy_update;
};

} // namespace binapi2::fapi::types
