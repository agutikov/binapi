// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <functional>
#include <vector>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class market_data_service
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    explicit market_data_service(client& owner) noexcept;

    [[nodiscard]] result<types::empty_response> ping();
    void ping(callback_type<types::empty_response> callback);
    [[nodiscard]] result<types::server_time_response> server_time();
    void server_time(callback_type<types::server_time_response> callback);
    [[nodiscard]] result<types::exchange_info_response> exchange_info(const types::exchange_info_request& request = {});
    void exchange_info(const types::exchange_info_request& request, callback_type<types::exchange_info_response> callback);
    [[nodiscard]] result<types::order_book_response> order_book(const types::order_book_request& request);
    void order_book(const types::order_book_request& request, callback_type<types::order_book_response> callback);
    [[nodiscard]] result<std::vector<types::recent_trade>> recent_trades(const types::recent_trades_request& request);
    void recent_trades(const types::recent_trades_request& request, callback_type<std::vector<types::recent_trade>> callback);
    [[nodiscard]] result<std::vector<types::aggregate_trade>> aggregate_trades(const types::aggregate_trades_request& request);
    void aggregate_trades(const types::aggregate_trades_request& request,
                          callback_type<std::vector<types::aggregate_trade>> callback);
    [[nodiscard]] result<std::vector<types::kline>> klines(const types::kline_request& request);
    void klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<std::vector<types::kline>> continuous_klines(const types::continuous_kline_request& request);
    void continuous_klines(const types::continuous_kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<std::vector<types::kline>> index_price_klines(const types::index_price_kline_request& request);
    void index_price_klines(const types::index_price_kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<std::vector<types::kline>> mark_price_klines(const types::kline_request& request);
    void mark_price_klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<std::vector<types::kline>> premium_index_klines(const types::kline_request& request);
    void premium_index_klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback);
    [[nodiscard]] result<types::book_ticker> book_ticker(const types::book_ticker_request& request);
    void book_ticker(const types::book_ticker_request& request, callback_type<types::book_ticker> callback);
    [[nodiscard]] result<std::vector<types::book_ticker>> book_tickers();
    void book_tickers(callback_type<std::vector<types::book_ticker>> callback);
    [[nodiscard]] result<types::price_ticker> price_ticker(const types::price_ticker_request& request);
    void price_ticker(const types::price_ticker_request& request, callback_type<types::price_ticker> callback);
    [[nodiscard]] result<std::vector<types::price_ticker>> price_tickers();
    void price_tickers(callback_type<std::vector<types::price_ticker>> callback);
    [[nodiscard]] result<types::ticker_24hr> ticker_24hr(const types::ticker_24hr_request& request);
    void ticker_24hr(const types::ticker_24hr_request& request, callback_type<types::ticker_24hr> callback);
    [[nodiscard]] result<std::vector<types::ticker_24hr>> ticker_24hrs();
    void ticker_24hrs(callback_type<std::vector<types::ticker_24hr>> callback);
    [[nodiscard]] result<types::mark_price> mark_price(const types::mark_price_request& request);
    void mark_price(const types::mark_price_request& request, callback_type<types::mark_price> callback);
    [[nodiscard]] result<std::vector<types::mark_price>> mark_prices();
    void mark_prices(callback_type<std::vector<types::mark_price>> callback);
    [[nodiscard]] result<std::vector<types::funding_rate_history_entry>> funding_rate_history(
        const types::funding_rate_history_request& request = {});
    void funding_rate_history(const types::funding_rate_history_request& request,
                              callback_type<std::vector<types::funding_rate_history_entry>> callback);
    [[nodiscard]] result<std::vector<types::funding_rate_info>> funding_rate_info();
    void funding_rate_info(callback_type<std::vector<types::funding_rate_info>> callback);
    [[nodiscard]] result<types::open_interest> open_interest(const types::open_interest_request& request);
    void open_interest(const types::open_interest_request& request, callback_type<types::open_interest> callback);
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
    [[nodiscard]] result<std::vector<types::recent_trade>> historical_trades(const types::historical_trades_request& request);
    void historical_trades(const types::historical_trades_request& request,
                           callback_type<std::vector<types::recent_trade>> callback);
    [[nodiscard]] result<std::vector<types::basis_entry>> basis(const types::basis_request& request);
    void basis(const types::basis_request& request, callback_type<std::vector<types::basis_entry>> callback);
    [[nodiscard]] result<types::price_ticker_v2> price_ticker_v2(const types::price_ticker_v2_request& request);
    void price_ticker_v2(const types::price_ticker_v2_request& request, callback_type<types::price_ticker_v2> callback);
    [[nodiscard]] result<std::vector<types::price_ticker_v2>> price_tickers_v2();
    void price_tickers_v2(callback_type<std::vector<types::price_ticker_v2>> callback);
    [[nodiscard]] result<std::vector<types::delivery_price_entry>> delivery_price(
        const types::delivery_price_request& request);
    void delivery_price(const types::delivery_price_request& request,
                        callback_type<std::vector<types::delivery_price_entry>> callback);
    [[nodiscard]] result<std::vector<types::composite_index_info>> composite_index_info(
        const types::composite_index_info_request& request);
    void composite_index_info(const types::composite_index_info_request& request,
                              callback_type<std::vector<types::composite_index_info>> callback);
    [[nodiscard]] result<types::index_constituents_response> index_constituents(
        const types::index_constituents_request& request);
    void index_constituents(const types::index_constituents_request& request,
                            callback_type<types::index_constituents_response> callback);
    [[nodiscard]] result<std::vector<types::asset_index_entry>> asset_index(const types::asset_index_request& request = {});
    void asset_index(const types::asset_index_request& request, callback_type<std::vector<types::asset_index_entry>> callback);
    [[nodiscard]] result<std::vector<types::insurance_fund_entry>> insurance_fund(
        const types::insurance_fund_request& request = {});
    void insurance_fund(const types::insurance_fund_request& request,
                        callback_type<std::vector<types::insurance_fund_entry>> callback);
    [[nodiscard]] result<std::vector<types::adl_risk_entry>> adl_risk(const types::adl_risk_request& request = {});
    void adl_risk(const types::adl_risk_request& request, callback_type<std::vector<types::adl_risk_entry>> callback);
    [[nodiscard]] result<std::vector<types::rpi_depth_entry>> rpi_depth(const types::rpi_depth_request& request);
    void rpi_depth(const types::rpi_depth_request& request, callback_type<std::vector<types::rpi_depth_entry>> callback);
    [[nodiscard]] result<std::vector<types::trading_schedule_entry>> trading_schedule(
        const types::trading_schedule_request& request = {});
    void trading_schedule(const types::trading_schedule_request& request,
                          callback_type<std::vector<types::trading_schedule_entry>> callback);

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
