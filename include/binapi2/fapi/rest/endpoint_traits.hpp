// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.
//
// Maps request types to their endpoint metadata and response types.
// Only defined for request types with a 1:1 endpoint mapping.
// Shared request types (futures_data_request_t, kline_request_t, download_id_request_t,
// download_link_request_t, batch_orders_request_t) are handled by named service methods.

/// @file
/// @brief Compile-time mapping from request types to endpoint metadata and response types.
///
/// Specializations of endpoint_traits exist only for request types that map to exactly
/// one endpoint. Request types shared across multiple endpoints (kline_request_t,
/// futures_data_request_t, download_id_request_t, download_link_request_t, batch_orders_request_t,
/// cancel_multiple_orders_request_t) have no specialization and must be dispatched through
/// named methods on the appropriate service class.

#pragma once

#include <binapi2/fapi/rest/generated_endpoints.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/convert.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/request_tags.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <vector>

namespace binapi2::fapi::rest {

/// @brief Primary template for endpoint traits; intentionally left undefined.
///
/// Each specialization must provide:
///   - @c response_type_t : the type to deserialize the JSON response body into.
///   - @c endpoint : a static constexpr reference to the endpoint_metadata instance
///     that describes the HTTP method, path, security, and signing requirements.
///
/// A specialization exists for each request struct with a unique 1:1 endpoint mapping.
template<class Request>
struct endpoint_traits;

// --- Market data ---

template<>
struct endpoint_traits<types::ping_request_t>
{
    using response_type_t = types::empty_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = ping_endpoint;
};

template<>
struct endpoint_traits<types::server_time_request_t>
{
    using response_type_t = types::server_time_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = server_time_endpoint;
};

template<>
struct endpoint_traits<types::exchange_info_request_t>
{
    using response_type_t = types::exchange_info_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = exchange_info_endpoint;
};

template<>
struct endpoint_traits<types::order_book_request_t>
{
    using response_type_t = types::order_book_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = order_book_endpoint;
};

template<>
struct endpoint_traits<types::recent_trades_request_t>
{
    using response_type_t = std::vector<types::recent_trade_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = recent_trades_endpoint;
};

template<>
struct endpoint_traits<types::aggregate_trades_request_t>
{
    using response_type_t = std::vector<types::aggregate_trade_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = aggregate_trades_endpoint;
};

template<>
struct endpoint_traits<types::continuous_kline_request_t>
{
    using response_type_t = std::vector<types::kline_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = continuous_klines_endpoint;
};

template<>
struct endpoint_traits<types::index_price_kline_request_t>
{
    using response_type_t = std::vector<types::kline_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = index_price_klines_endpoint;
};

template<>
struct endpoint_traits<types::book_ticker_request_t>
{
    using response_type_t = types::book_ticker_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = book_ticker_endpoint;
};

template<>
struct endpoint_traits<types::price_ticker_request_t>
{
    using response_type_t = types::price_ticker_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = price_ticker_endpoint;
};

template<>
struct endpoint_traits<types::ticker_24hr_request_t>
{
    using response_type_t = types::ticker_24hr_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = ticker_24hr_endpoint;
};

template<>
struct endpoint_traits<types::mark_price_request_t>
{
    using response_type_t = types::mark_price_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = mark_price_endpoint;
};

template<>
struct endpoint_traits<types::funding_rate_history_request_t>
{
    using response_type_t = std::vector<types::funding_rate_history_entry_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = funding_rate_history_endpoint;
};

template<>
struct endpoint_traits<types::open_interest_request_t>
{
    using response_type_t = types::open_interest_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = open_interest_endpoint;
};

template<>
struct endpoint_traits<types::historical_trades_request_t>
{
    using response_type_t = std::vector<types::recent_trade_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = historical_trades_endpoint;
};

template<>
struct endpoint_traits<types::basis_request_t>
{
    using response_type_t = std::vector<types::basis_entry_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = basis_endpoint;
};

template<>
struct endpoint_traits<types::price_ticker_v2_request_t>
{
    using response_type_t = types::price_ticker_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = price_ticker_v2_endpoint;
};

template<>
struct endpoint_traits<types::delivery_price_request_t>
{
    using response_type_t = std::vector<types::delivery_price_entry_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = delivery_price_endpoint;
};

template<>
struct endpoint_traits<types::composite_index_info_request_t>
{
    using response_type_t = std::vector<types::composite_index_info_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = composite_index_info_endpoint;
};

template<>
struct endpoint_traits<types::index_constituents_request_t>
{
    using response_type_t = types::index_constituents_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = index_constituents_endpoint;
};

template<>
struct endpoint_traits<types::asset_index_request_t>
{
    using response_type_t = std::vector<types::asset_index_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = asset_index_endpoint;
};

template<>
struct endpoint_traits<types::insurance_fund_request_t>
{
    using response_type_t = types::insurance_fund_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = insurance_fund_endpoint;
};
template<>
struct endpoint_traits<types::insurance_funds_request_t>
{
    using response_type_t = std::vector<types::insurance_fund_response_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = insurance_fund_endpoint;
};

template<>
struct endpoint_traits<types::adl_risk_request_t>
{
    using response_type_t = std::vector<types::adl_risk_entry_t>;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = adl_risk_endpoint;
};

template<>
struct endpoint_traits<types::rpi_depth_request_t>
{
    using response_type_t = types::order_book_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = rpi_depth_endpoint;
};

template<>
struct endpoint_traits<types::trading_schedule_request_t>
{
    using response_type_t = types::trading_schedule_response_t;
    using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = trading_schedule_endpoint;
};

// --- Account ---

template<>
struct endpoint_traits<types::position_risk_request_t>
{
    using response_type_t = std::vector<types::position_risk_t>;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = position_risk_endpoint;
};

template<>
struct endpoint_traits<types::symbol_config_request_t>
{
    using response_type_t = std::vector<types::symbol_config_entry_t>;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = symbol_config_endpoint;
};

template<>
struct endpoint_traits<types::income_history_request_t>
{
    using response_type_t = std::vector<types::income_history_entry_t>;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = income_history_endpoint;
};

template<>
struct endpoint_traits<types::leverage_bracket_request_t>
{
    using response_type_t = std::vector<types::symbol_leverage_brackets_t>;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = leverage_brackets_endpoint;
};

template<>
struct endpoint_traits<types::commission_rate_request_t>
{
    using response_type_t = types::commission_rate_response_t;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = commission_rate_endpoint;
};

template<>
struct endpoint_traits<types::toggle_bnb_burn_request_t>
{
    using response_type_t = types::bnb_burn_status_response_t;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = toggle_bnb_burn_endpoint;
};

template<>
struct endpoint_traits<types::quantitative_rules_request_t>
{
    using response_type_t = types::quantitative_rules_response_t;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = quantitative_rules_endpoint;
};

template<>
struct endpoint_traits<types::pm_account_info_request_t>
{
    using response_type_t = types::pm_account_info_response_t;
    using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = pm_account_info_endpoint;
};

// --- Trade ---

template<>
struct endpoint_traits<types::new_order_request_t>
{
    using response_type_t = types::order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = new_order_endpoint;
};

template<>
struct endpoint_traits<types::modify_order_request_t>
{
    using response_type_t = types::order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = modify_order_endpoint;
};

template<>
struct endpoint_traits<types::cancel_order_request_t>
{
    using response_type_t = types::order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = cancel_order_endpoint;
};

template<>
struct endpoint_traits<types::query_order_request_t>
{
    using response_type_t = types::order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = query_order_endpoint;
};

template<>
struct endpoint_traits<types::cancel_all_open_orders_request_t>
{
    using response_type_t = types::code_msg_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = cancel_all_open_orders_endpoint;
};

template<>
struct endpoint_traits<types::auto_cancel_request_t>
{
    using response_type_t = types::auto_cancel_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = auto_cancel_endpoint;
};

template<>
struct endpoint_traits<types::query_open_order_request_t>
{
    using response_type_t = types::order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = query_open_order_endpoint;
};

template<>
struct endpoint_traits<types::all_open_orders_request_t>
{
    using response_type_t = std::vector<types::order_response_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = all_open_orders_endpoint;
};

template<>
struct endpoint_traits<types::all_orders_request_t>
{
    using response_type_t = std::vector<types::order_response_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = all_orders_endpoint;
};

template<>
struct endpoint_traits<types::position_info_v3_request_t>
{
    using response_type_t = std::vector<types::position_risk_v3_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = position_risk_v3_endpoint;
};

template<>
struct endpoint_traits<types::adl_quantile_request_t>
{
    using response_type_t = std::vector<types::adl_quantile_entry_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = adl_quantile_endpoint;
};

template<>
struct endpoint_traits<types::force_orders_request_t>
{
    using response_type_t = std::vector<types::order_response_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = force_orders_endpoint;
};

template<>
struct endpoint_traits<types::account_trade_request_t>
{
    using response_type_t = std::vector<types::account_trade_entry_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = account_trades_endpoint;
};

template<>
struct endpoint_traits<types::change_position_mode_request_t>
{
    using response_type_t = types::code_msg_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = change_position_mode_endpoint;
};

template<>
struct endpoint_traits<types::change_multi_assets_mode_request_t>
{
    using response_type_t = types::code_msg_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = change_multi_assets_endpoint;
};

template<>
struct endpoint_traits<types::change_leverage_request_t>
{
    using response_type_t = types::change_leverage_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = change_leverage_endpoint;
};

template<>
struct endpoint_traits<types::change_margin_type_request_t>
{
    using response_type_t = types::code_msg_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = change_margin_type_endpoint;
};

template<>
struct endpoint_traits<types::modify_isolated_margin_request_t>
{
    using response_type_t = types::modify_isolated_margin_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = modify_isolated_margin_endpoint;
};

template<>
struct endpoint_traits<types::position_margin_history_request_t>
{
    using response_type_t = std::vector<types::position_margin_history_entry_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = position_margin_history_endpoint;
};

template<>
struct endpoint_traits<types::order_modify_history_request_t>
{
    using response_type_t = std::vector<types::order_response_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = order_modify_history_endpoint;
};

template<>
struct endpoint_traits<types::new_algo_order_request_t>
{
    using response_type_t = types::algo_order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = new_algo_order_endpoint;
};

template<>
struct endpoint_traits<types::cancel_algo_order_request_t>
{
    using response_type_t = types::code_msg_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = cancel_algo_order_endpoint;
};

template<>
struct endpoint_traits<types::query_algo_order_request_t>
{
    using response_type_t = types::algo_order_response_t;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = query_algo_order_endpoint;
};

template<>
struct endpoint_traits<types::all_algo_orders_request_t>
{
    using response_type_t = std::vector<types::algo_order_response_t>;
    using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = all_algo_orders_endpoint;
};

// --- Convert ---

template<>
struct endpoint_traits<types::convert_quote_request_t>
{
    using response_type_t = types::convert_quote_response_t;
    using service_tag = types::rest_convert_tag;
    static constexpr auto& endpoint = convert_get_quote_endpoint;
};

template<>
struct endpoint_traits<types::convert_accept_request_t>
{
    using response_type_t = types::convert_accept_response_t;
    using service_tag = types::rest_convert_tag;
    static constexpr auto& endpoint = convert_accept_quote_endpoint;
};

template<>
struct endpoint_traits<types::convert_order_status_request_t>
{
    using response_type_t = types::convert_order_status_response_t;
    using service_tag = types::rest_convert_tag;
    static constexpr auto& endpoint = convert_order_status_endpoint;
};

// --- New: parameterless market data ---

template<> struct endpoint_traits<types::book_tickers_request_t> {
    using response_type_t = std::vector<types::book_ticker_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = book_ticker_endpoint; };
template<> struct endpoint_traits<types::price_tickers_request_t> {
    using response_type_t = std::vector<types::price_ticker_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = price_ticker_endpoint; };
template<> struct endpoint_traits<types::price_tickers_v2_request_t> {
    using response_type_t = std::vector<types::price_ticker_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = price_ticker_v2_endpoint; };
template<> struct endpoint_traits<types::ticker_24hrs_request_t> {
    using response_type_t = std::vector<types::ticker_24hr_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = ticker_24hr_endpoint; };
template<> struct endpoint_traits<types::mark_prices_request_t> {
    using response_type_t = std::vector<types::mark_price_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = mark_price_endpoint; };
template<> struct endpoint_traits<types::funding_rate_info_request_t> {
    using response_type_t = std::vector<types::funding_rate_info_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = funding_rate_info_endpoint; };

// --- New: distinct kline request types ---

template<> struct endpoint_traits<types::klines_request_t> {
    using response_type_t = std::vector<types::kline_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = klines_endpoint; };
template<> struct endpoint_traits<types::mark_price_klines_request_t> {
    using response_type_t = std::vector<types::kline_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = mark_price_klines_endpoint; };
template<> struct endpoint_traits<types::premium_index_klines_request_t> {
    using response_type_t = std::vector<types::kline_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = premium_index_klines_endpoint; };

// --- New: distinct futures data request types ---

template<> struct endpoint_traits<types::open_interest_statistics_request_t> {
    using response_type_t = std::vector<types::open_interest_statistics_entry_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = open_interest_statistics_endpoint; };
template<> struct endpoint_traits<types::top_long_short_account_ratio_request_t> {
    using response_type_t = std::vector<types::long_short_ratio_entry_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = top_long_short_account_ratio_endpoint; };
template<> struct endpoint_traits<types::top_trader_long_short_ratio_request_t> {
    using response_type_t = std::vector<types::long_short_ratio_entry_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = top_trader_long_short_ratio_endpoint; };
template<> struct endpoint_traits<types::long_short_ratio_request_t> {
    using response_type_t = std::vector<types::long_short_ratio_entry_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = long_short_ratio_endpoint; };
template<> struct endpoint_traits<types::taker_buy_sell_volume_request_t> {
    using response_type_t = std::vector<types::taker_buy_sell_volume_entry_t>; using service_tag = types::rest_market_data_tag;
    static constexpr auto& endpoint = taker_buy_sell_volume_endpoint; };

// --- New: parameterless account ---

template<> struct endpoint_traits<types::account_information_request_t> {
    using response_type_t = types::account_information_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = account_information_endpoint; };
template<> struct endpoint_traits<types::balances_request_t> {
    using response_type_t = std::vector<types::futures_account_balance_t>; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = account_balances_endpoint; };
template<> struct endpoint_traits<types::account_config_request_t> {
    using response_type_t = types::account_config_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = account_config_endpoint; };
template<> struct endpoint_traits<types::get_multi_assets_mode_request_t> {
    using response_type_t = types::multi_assets_mode_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = get_multi_assets_mode_endpoint; };
template<> struct endpoint_traits<types::get_position_mode_request_t> {
    using response_type_t = types::position_mode_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = get_position_mode_endpoint; };
template<> struct endpoint_traits<types::rate_limit_order_request_t> {
    using response_type_t = std::vector<types::rate_limit_t>; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = rate_limit_order_endpoint; };
template<> struct endpoint_traits<types::get_bnb_burn_request_t> {
    using response_type_t = types::bnb_burn_status_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = get_bnb_burn_endpoint; };

// --- New: distinct download request types ---

template<> struct endpoint_traits<types::download_id_transaction_request_t> {
    using response_type_t = types::download_id_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = download_id_transaction_endpoint; };
template<> struct endpoint_traits<types::download_link_transaction_request_t> {
    using response_type_t = types::download_link_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = download_link_transaction_endpoint; };
template<> struct endpoint_traits<types::download_id_order_request_t> {
    using response_type_t = types::download_id_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = download_id_order_endpoint; };
template<> struct endpoint_traits<types::download_link_order_request_t> {
    using response_type_t = types::download_link_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = download_link_order_endpoint; };
template<> struct endpoint_traits<types::download_id_trade_request_t> {
    using response_type_t = types::download_id_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = download_id_trade_endpoint; };
template<> struct endpoint_traits<types::download_link_trade_request_t> {
    using response_type_t = types::download_link_response_t; using service_tag = types::rest_account_tag;
    static constexpr auto& endpoint = download_link_trade_endpoint; };

// --- New: parameterless trade ---

template<> struct endpoint_traits<types::test_order_request_t> {
    using response_type_t = types::order_response_t; using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = test_order_endpoint; };
template<> struct endpoint_traits<types::open_algo_orders_request_t> {
    using response_type_t = std::vector<types::algo_order_response_t>; using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = open_algo_orders_endpoint; };
template<> struct endpoint_traits<types::cancel_all_algo_orders_request_t> {
    using response_type_t = types::code_msg_response_t; using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = cancel_all_algo_orders_endpoint; };
template<> struct endpoint_traits<types::tradfi_perps_request_t> {
    using response_type_t = types::code_msg_response_t; using service_tag = types::rest_trade_tag;
    static constexpr auto& endpoint = tradfi_perps_endpoint; };

// --- New: user data stream ---

template<> struct endpoint_traits<types::start_listen_key_request_t> {
    using response_type_t = types::listen_key_response_t; using service_tag = types::rest_user_data_stream_tag;
    static constexpr auto& endpoint = start_listen_key_endpoint; };
template<> struct endpoint_traits<types::keepalive_listen_key_request_t> {
    using response_type_t = types::listen_key_response_t; using service_tag = types::rest_user_data_stream_tag;
    static constexpr auto& endpoint = keepalive_listen_key_endpoint; };
template<> struct endpoint_traits<types::close_listen_key_request_t> {
    using response_type_t = types::empty_response_t; using service_tag = types::rest_user_data_stream_tag;
    static constexpr auto& endpoint = close_listen_key_endpoint; };

// --- Concepts ---

/// @brief Concept satisfied by request types that have a valid endpoint_traits specialization.
///
/// Requires that endpoint_traits<T> defines a response_type_t alias and an endpoint
/// member convertible to const endpoint_metadata&.
template<class T>
concept has_endpoint_traits = requires {
    typename endpoint_traits<T>::response_type_t;
    typename endpoint_traits<T>::service_tag;
    { endpoint_traits<T>::endpoint } -> std::convertible_to<const endpoint_metadata&>;
};

template<class T>
concept is_market_data_request = has_endpoint_traits<T>
    && std::same_as<typename endpoint_traits<T>::service_tag, types::rest_market_data_tag>;

template<class T>
concept is_account_request = has_endpoint_traits<T>
    && std::same_as<typename endpoint_traits<T>::service_tag, types::rest_account_tag>;

template<class T>
concept is_trade_request = has_endpoint_traits<T>
    && std::same_as<typename endpoint_traits<T>::service_tag, types::rest_trade_tag>;

template<class T>
concept is_convert_request = has_endpoint_traits<T>
    && std::same_as<typename endpoint_traits<T>::service_tag, types::rest_convert_tag>;

template<class T>
concept is_user_data_stream_request = has_endpoint_traits<T>
    && std::same_as<typename endpoint_traits<T>::service_tag, types::rest_user_data_stream_tag>;

} // namespace binapi2::fapi::rest
