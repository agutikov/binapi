// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Market data service for USD-M Futures public endpoints.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <boost/cobalt/task.hpp>

#include <vector>

namespace binapi2::fapi::rest {

/// @brief Service group for public market data endpoints.
///
/// Request types with a unique endpoint mapping (e.g. ping_request_t, order_book_request_t)
/// are exposed as using declarations and can be dispatched through the inherited
/// execute/async_execute template methods from service.
///
/// Named methods are provided for two categories:
///   - **Shared request types**: kline_request_t is used by three different kline_t endpoints
///     (klines, mark_price_klines, premium_index_klines), and futures_data_request_t is
///     used by five statistical endpoints. Since the request type alone cannot identify
///     the endpoint, these require explicit named methods.
///   - **Parameterless list endpoints**: book_tickers(), price_tickers(), etc. return
///     the full collection without requiring a request struct.
class market_data_service : public service
{
public:
    using service::service;

    // Request types with 1:1 endpoint mapping (use execute/async_execute).
    using ping_request_t = types::ping_request_t;
    using server_time_request_t = types::server_time_request_t;
    using exchange_info_request_t = types::exchange_info_request_t;
    using order_book_request_t = types::order_book_request_t;
    using recent_trades_request_t = types::recent_trades_request_t;
    using aggregate_trades_request_t = types::aggregate_trades_request_t;
    using continuous_kline_request_t = types::continuous_kline_request_t;
    using index_price_kline_request_t = types::index_price_kline_request_t;
    using book_ticker_request_t = types::book_ticker_request_t;
    using price_ticker_request_t = types::price_ticker_request_t;
    using ticker_24hr_request_t = types::ticker_24hr_request_t;
    using mark_price_request_t = types::mark_price_request_t;
    using funding_rate_history_request_t = types::funding_rate_history_request_t;
    using open_interest_request_t = types::open_interest_request_t;
    using historical_trades_request_t = types::historical_trades_request_t;
    using basis_request_t = types::basis_request_t;
    using price_ticker_v2_request_t = types::price_ticker_v2_request_t;
    using delivery_price_request_t = types::delivery_price_request_t;
    using composite_index_info_request_t = types::composite_index_info_request_t;
    using index_constituents_request_t = types::index_constituents_request_t;
    using asset_index_request_t = types::asset_index_request_t;
    using insurance_fund_request_t = types::insurance_fund_request_t;
    using adl_risk_request_t = types::adl_risk_request_t;
    using rpi_depth_request_t = types::rpi_depth_request_t;
    using trading_schedule_request_t = types::trading_schedule_request_t;

    /// @brief Fetch standard klines (candlestick data) for a symbol.
    /// @param request  Kline parameters (symbol, interval, limit, time range).
    /// @brief Async variant of klines.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::kline_t>>> async_klines(const types::kline_request_t& request);

    /// @brief Fetch mark price klines for a symbol.
    /// @param request  Kline parameters (same type as klines; routed to the mark price kline_t endpoint).
    /// @brief Async variant of mark_price_klines.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::kline_t>>> async_mark_price_klines(const types::kline_request_t& request);

    /// @brief Fetch premium index klines for a symbol.
    /// @param request  Kline parameters (same type; routed to the premium index kline_t endpoint).
    /// @brief Async variant of premium_index_klines.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::kline_t>>> async_premium_index_klines(const types::kline_request_t& request);

    /// @brief Fetch open interest statistics (historical open interest).
    /// @param request  Futures data parameters (symbol, period, limit, time range).
    [[nodiscard]] result<std::vector<types::open_interest_statistics_entry_t>> open_interest_statistics(
        const types::futures_data_request_t& request);
    /// @brief Async variant of open_interest_statistics.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::open_interest_statistics_entry_t>>> async_open_interest_statistics(
        const types::futures_data_request_t& request);

    /// @brief Fetch top trader long/short account ratio.
    /// @param request  Futures data parameters (same shared type; routed to the account ratio endpoint).
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry_t>> top_long_short_account_ratio(
        const types::futures_data_request_t& request);
    /// @brief Async variant of top_long_short_account_ratio.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::long_short_ratio_entry_t>>> async_top_long_short_account_ratio(
        const types::futures_data_request_t& request);

    /// @brief Fetch top trader long/short position ratio.
    /// @param request  Futures data parameters.
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry_t>> top_trader_long_short_ratio(
        const types::futures_data_request_t& request);
    /// @brief Async variant of top_trader_long_short_ratio.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::long_short_ratio_entry_t>>> async_top_trader_long_short_ratio(
        const types::futures_data_request_t& request);

    /// @brief Fetch global long/short account ratio.
    /// @param request  Futures data parameters.
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry_t>> long_short_ratio(
        const types::futures_data_request_t& request);
    /// @brief Async variant of long_short_ratio.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::long_short_ratio_entry_t>>> async_long_short_ratio(
        const types::futures_data_request_t& request);

    /// @brief Fetch taker buy/sell volume ratio.
    /// @param request  Futures data parameters.
    [[nodiscard]] result<std::vector<types::taker_buy_sell_volume_entry_t>> taker_buy_sell_volume(
        const types::futures_data_request_t& request);
    /// @brief Async variant of taker_buy_sell_volume.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::taker_buy_sell_volume_entry_t>>> async_taker_buy_sell_volume(
        const types::futures_data_request_t& request);

    /// @brief Fetch best book ticker for all symbols.
    /// @brief Async variant of book_tickers.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::book_ticker_t>>> async_book_tickers();

    /// @brief Fetch latest price for all symbols (v1).
    /// @brief Async variant of price_tickers.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::price_ticker_t>>> async_price_tickers();

    /// @brief Fetch latest price for all symbols (v2).
    /// @brief Async variant of price_tickers_v2.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::price_ticker_t>>> async_price_tickers_v2();

    /// @brief Fetch 24-hour rolling statistics for all symbols.
    /// @brief Async variant of ticker_24hrs.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::ticker_24hr_t>>> async_ticker_24hrs();

    /// @brief Fetch mark price and funding rate for all symbols.
    /// @brief Async variant of mark_prices.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::mark_price_t>>> async_mark_prices();

    /// @brief Fetch funding rate information for all symbols.
    /// @brief Async variant of funding_rate_info_t.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::funding_rate_info_t>>> async_funding_rate_info();
};

} // namespace binapi2::fapi::rest
