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
/// Request types with a unique endpoint mapping (e.g. ping_request, order_book_request)
/// are exposed as using declarations and can be dispatched through the inherited
/// execute/async_execute template methods from service.
///
/// Named methods are provided for two categories:
///   - **Shared request types**: kline_request is used by three different kline endpoints
///     (klines, mark_price_klines, premium_index_klines), and futures_data_request is
///     used by five statistical endpoints. Since the request type alone cannot identify
///     the endpoint, these require explicit named methods.
///   - **Parameterless list endpoints**: book_tickers(), price_tickers(), etc. return
///     the full collection without requiring a request struct.
class market_data_service : public service
{
public:
    using service::service;

    // Request types with 1:1 endpoint mapping (use execute/async_execute).
    using ping_request = types::ping_request;
    using server_time_request = types::server_time_request;
    using exchange_info_request = types::exchange_info_request;
    using order_book_request = types::order_book_request;
    using recent_trades_request = types::recent_trades_request;
    using aggregate_trades_request = types::aggregate_trades_request;
    using continuous_kline_request = types::continuous_kline_request;
    using index_price_kline_request = types::index_price_kline_request;
    using book_ticker_request = types::book_ticker_request;
    using price_ticker_request = types::price_ticker_request;
    using ticker_24hr_request = types::ticker_24hr_request;
    using mark_price_request = types::mark_price_request;
    using funding_rate_history_request = types::funding_rate_history_request;
    using open_interest_request = types::open_interest_request;
    using historical_trades_request = types::historical_trades_request;
    using basis_request = types::basis_request;
    using price_ticker_v2_request = types::price_ticker_v2_request;
    using delivery_price_request = types::delivery_price_request;
    using composite_index_info_request = types::composite_index_info_request;
    using index_constituents_request = types::index_constituents_request;
    using asset_index_request = types::asset_index_request;
    using insurance_fund_request = types::insurance_fund_request;
    using adl_risk_request = types::adl_risk_request;
    using rpi_depth_request = types::rpi_depth_request;
    using trading_schedule_request = types::trading_schedule_request;

    /// @brief Fetch standard klines (candlestick data) for a symbol.
    /// @param request  Kline parameters (symbol, interval, limit, time range).
    [[nodiscard]] result<std::vector<types::kline>> klines(const types::kline_request& request);
    /// @brief Async variant of klines.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::kline>>> async_klines(const types::kline_request& request);

    /// @brief Fetch mark price klines for a symbol.
    /// @param request  Kline parameters (same type as klines; routed to the mark price kline endpoint).
    [[nodiscard]] result<std::vector<types::kline>> mark_price_klines(const types::kline_request& request);
    /// @brief Async variant of mark_price_klines.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::kline>>> async_mark_price_klines(const types::kline_request& request);

    /// @brief Fetch premium index klines for a symbol.
    /// @param request  Kline parameters (same type; routed to the premium index kline endpoint).
    [[nodiscard]] result<std::vector<types::kline>> premium_index_klines(const types::kline_request& request);
    /// @brief Async variant of premium_index_klines.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::kline>>> async_premium_index_klines(const types::kline_request& request);

    /// @brief Fetch open interest statistics (historical open interest).
    /// @param request  Futures data parameters (symbol, period, limit, time range).
    [[nodiscard]] result<std::vector<types::open_interest_statistics_entry>> open_interest_statistics(
        const types::futures_data_request& request);
    /// @brief Async variant of open_interest_statistics.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::open_interest_statistics_entry>>> async_open_interest_statistics(
        const types::futures_data_request& request);

    /// @brief Fetch top trader long/short account ratio.
    /// @param request  Futures data parameters (same shared type; routed to the account ratio endpoint).
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> top_long_short_account_ratio(
        const types::futures_data_request& request);
    /// @brief Async variant of top_long_short_account_ratio.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::long_short_ratio_entry>>> async_top_long_short_account_ratio(
        const types::futures_data_request& request);

    /// @brief Fetch top trader long/short position ratio.
    /// @param request  Futures data parameters.
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> top_trader_long_short_ratio(
        const types::futures_data_request& request);
    /// @brief Async variant of top_trader_long_short_ratio.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::long_short_ratio_entry>>> async_top_trader_long_short_ratio(
        const types::futures_data_request& request);

    /// @brief Fetch global long/short account ratio.
    /// @param request  Futures data parameters.
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> long_short_ratio(
        const types::futures_data_request& request);
    /// @brief Async variant of long_short_ratio.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::long_short_ratio_entry>>> async_long_short_ratio(
        const types::futures_data_request& request);

    /// @brief Fetch taker buy/sell volume ratio.
    /// @param request  Futures data parameters.
    [[nodiscard]] result<std::vector<types::taker_buy_sell_volume_entry>> taker_buy_sell_volume(
        const types::futures_data_request& request);
    /// @brief Async variant of taker_buy_sell_volume.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::taker_buy_sell_volume_entry>>> async_taker_buy_sell_volume(
        const types::futures_data_request& request);

    /// @brief Fetch best book ticker for all symbols.
    [[nodiscard]] result<std::vector<types::book_ticker>> book_tickers();
    /// @brief Async variant of book_tickers.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::book_ticker>>> async_book_tickers();

    /// @brief Fetch latest price for all symbols (v1).
    [[nodiscard]] result<std::vector<types::price_ticker>> price_tickers();
    /// @brief Async variant of price_tickers.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::price_ticker>>> async_price_tickers();

    /// @brief Fetch latest price for all symbols (v2).
    [[nodiscard]] result<std::vector<types::price_ticker>> price_tickers_v2();
    /// @brief Async variant of price_tickers_v2.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::price_ticker>>> async_price_tickers_v2();

    /// @brief Fetch 24-hour rolling statistics for all symbols.
    [[nodiscard]] result<std::vector<types::ticker_24hr>> ticker_24hrs();
    /// @brief Async variant of ticker_24hrs.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::ticker_24hr>>> async_ticker_24hrs();

    /// @brief Fetch mark price and funding rate for all symbols.
    [[nodiscard]] result<std::vector<types::mark_price>> mark_prices();
    /// @brief Async variant of mark_prices.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::mark_price>>> async_mark_prices();

    /// @brief Fetch funding rate information for all symbols.
    [[nodiscard]] result<std::vector<types::funding_rate_info>> funding_rate_info();
    /// @brief Async variant of funding_rate_info.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::funding_rate_info>>> async_funding_rate_info();
};

} // namespace binapi2::fapi::rest
