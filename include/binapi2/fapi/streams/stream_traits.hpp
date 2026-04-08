// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_traits.hpp
/// @brief Compile-time mapping from subscription types to stream target paths and event types.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/user_stream_events.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <algorithm>
#include <string>

namespace binapi2::fapi::streams {

template<class Subscription>
struct stream_traits;

namespace detail {

inline std::string to_lower(std::string s)
{
    std::ranges::transform(s, s.begin(), ::tolower);
    return s;
}

} // namespace detail

// --- Individual symbol streams ---

template<> struct stream_traits<types::aggregate_trade_subscription> {
    using event_type = types::aggregate_trade_stream_event_t;
    static std::string target(const config& cfg, const types::aggregate_trade_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@aggTrade"; }
};

template<> struct stream_traits<types::book_ticker_subscription> {
    using event_type = types::book_ticker_stream_event_t;
    static std::string target(const config& cfg, const types::book_ticker_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@bookTicker"; }
};

template<> struct stream_traits<types::mark_price_subscription> {
    using event_type = types::mark_price_stream_event_t;
    static std::string target(const config& cfg, const types::mark_price_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + (sub.every_1s ? "@markPrice@1s" : "@markPrice"); }
};

template<> struct stream_traits<types::mini_ticker_subscription> {
    using event_type = types::mini_ticker_stream_event_t;
    static std::string target(const config& cfg, const types::mini_ticker_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@miniTicker"; }
};

template<> struct stream_traits<types::ticker_subscription> {
    using event_type = types::ticker_stream_event_t;
    static std::string target(const config& cfg, const types::ticker_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@ticker"; }
};

template<> struct stream_traits<types::diff_book_depth_subscription> {
    using event_type = types::depth_stream_event_t;
    static std::string target(const config& cfg, const types::diff_book_depth_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@depth@" + sub.speed; }
};

template<> struct stream_traits<types::partial_book_depth_subscription> {
    using event_type = types::depth_stream_event_t;
    static std::string target(const config& cfg, const types::partial_book_depth_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@depth" + std::to_string(sub.levels) + "@" + sub.speed; }
};

template<> struct stream_traits<types::kline_subscription> {
    using event_type = types::kline_stream_event_t;
    static std::string target(const config& cfg, const types::kline_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@kline_" + types::to_string(sub.interval); }
};

template<> struct stream_traits<types::liquidation_order_subscription> {
    using event_type = types::liquidation_order_stream_event_t;
    static std::string target(const config& cfg, const types::liquidation_order_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@forceOrder"; }
};

template<> struct stream_traits<types::composite_index_subscription> {
    using event_type = types::composite_index_stream_event_t;
    static std::string target(const config& cfg, const types::composite_index_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@compositeIndex"; }
};

template<> struct stream_traits<types::asset_index_subscription> {
    using event_type = types::asset_index_stream_event_t;
    static std::string target(const config& cfg, const types::asset_index_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@assetIndex"; }
};

template<> struct stream_traits<types::rpi_diff_book_depth_subscription> {
    using event_type = types::depth_stream_event_t;
    static std::string target(const config& cfg, const types::rpi_diff_book_depth_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.symbol) + "@rpiDepth"; }
};

// --- All-market streams ---

template<> struct stream_traits<types::all_market_mini_ticker_subscription> {
    using event_type = std::vector<types::mini_ticker_stream_event_t>;
    static std::string target(const config& cfg, const types::all_market_mini_ticker_subscription&)
    { return cfg.stream_base_target + "/!miniTicker@arr"; }
};

template<> struct stream_traits<types::all_market_ticker_subscription> {
    using event_type = std::vector<types::ticker_stream_event_t>;
    static std::string target(const config& cfg, const types::all_market_ticker_subscription&)
    { return cfg.stream_base_target + "/!ticker@arr"; }
};

template<> struct stream_traits<types::all_book_ticker_subscription> {
    using event_type = types::book_ticker_stream_event_t;
    static std::string target(const config& cfg, const types::all_book_ticker_subscription&)
    { return cfg.stream_base_target + "/!bookTicker"; }
};

template<> struct stream_traits<types::all_market_liquidation_order_subscription> {
    using event_type = types::liquidation_order_stream_event_t;
    static std::string target(const config& cfg, const types::all_market_liquidation_order_subscription&)
    { return cfg.stream_base_target + "/!forceOrder@arr"; }
};

template<> struct stream_traits<types::all_market_mark_price_subscription> {
    using event_type = std::vector<types::mark_price_stream_event_t>;
    static std::string target(const config& cfg, const types::all_market_mark_price_subscription& sub)
    { return cfg.stream_base_target + (sub.every_1s ? "/!markPrice@arr@1s" : "/!markPrice@arr"); }
};

template<> struct stream_traits<types::contract_info_subscription> {
    using event_type = types::contract_info_stream_event_t;
    static std::string target(const config& cfg, const types::contract_info_subscription&)
    { return cfg.stream_base_target + "/!contractInfo"; }
};

template<> struct stream_traits<types::all_asset_index_subscription> {
    using event_type = types::all_asset_index_stream_event;
    static std::string target(const config& cfg, const types::all_asset_index_subscription&)
    { return cfg.stream_base_target + "/!assetIndex@arr"; }
};

template<> struct stream_traits<types::trading_session_subscription> {
    using event_type = types::trading_session_stream_event_t;
    static std::string target(const config& cfg, const types::trading_session_subscription&)
    { return cfg.stream_base_target + "/!tradingSession"; }
};

// --- Continuous contract ---

template<> struct stream_traits<types::continuous_contract_kline_subscription> {
    using event_type = types::continuous_contract_kline_stream_event_t;
    static std::string target(const config& cfg, const types::continuous_contract_kline_subscription& sub)
    { return cfg.stream_base_target + "/" + detail::to_lower(sub.pair) + "_" + types::to_string(sub.contract_type)
        + "@continuousKline_" + types::to_string(sub.interval); }
};

// --- Concept ---

template<class T>
concept has_stream_traits = requires {
    typename stream_traits<T>::event_type;
};

} // namespace binapi2::fapi::streams
