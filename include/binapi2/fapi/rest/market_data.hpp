// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <vector>

namespace binapi2::fapi::rest {

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

    // Methods for shared request types (kline_request used by 3 endpoints,
    // futures_data_request used by 5 endpoints).
    [[nodiscard]] result<std::vector<types::kline>> klines(const types::kline_request& request);
    void klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<std::vector<types::kline>> mark_price_klines(const types::kline_request& request);
    void mark_price_klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<std::vector<types::kline>> premium_index_klines(const types::kline_request& request);
    void premium_index_klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback);

    [[nodiscard]] result<std::vector<types::open_interest_statistics_entry>> open_interest_statistics(
        const types::futures_data_request& request);
    void open_interest_statistics(const types::futures_data_request& request,
                                  callback_type<std::vector<types::open_interest_statistics_entry>> callback);
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> top_long_short_account_ratio(
        const types::futures_data_request& request);
    void top_long_short_account_ratio(const types::futures_data_request& request,
                                      callback_type<std::vector<types::long_short_ratio_entry>> callback);
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> top_trader_long_short_ratio(
        const types::futures_data_request& request);
    void top_trader_long_short_ratio(const types::futures_data_request& request,
                                     callback_type<std::vector<types::long_short_ratio_entry>> callback);
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> long_short_ratio(
        const types::futures_data_request& request);
    void long_short_ratio(const types::futures_data_request& request,
                          callback_type<std::vector<types::long_short_ratio_entry>> callback);
    [[nodiscard]] result<std::vector<types::taker_buy_sell_volume_entry>> taker_buy_sell_volume(
        const types::futures_data_request& request);
    void taker_buy_sell_volume(const types::futures_data_request& request,
                               callback_type<std::vector<types::taker_buy_sell_volume_entry>> callback);

    // Parameterless list endpoints (no request type).
    [[nodiscard]] result<std::vector<types::book_ticker>> book_tickers();
    void book_tickers(callback_type<std::vector<types::book_ticker>> callback);
    [[nodiscard]] result<std::vector<types::price_ticker>> price_tickers();
    void price_tickers(callback_type<std::vector<types::price_ticker>> callback);
    [[nodiscard]] result<std::vector<types::price_ticker>> price_tickers_v2();
    void price_tickers_v2(callback_type<std::vector<types::price_ticker>> callback);
    [[nodiscard]] result<std::vector<types::ticker_24hr>> ticker_24hrs();
    void ticker_24hrs(callback_type<std::vector<types::ticker_24hr>> callback);
    [[nodiscard]] result<std::vector<types::mark_price>> mark_prices();
    void mark_prices(callback_type<std::vector<types::mark_price>> callback);
    [[nodiscard]] result<std::vector<types::funding_rate_info>> funding_rate_info();
    void funding_rate_info(callback_type<std::vector<types::funding_rate_info>> callback);
};

} // namespace binapi2::fapi::rest
