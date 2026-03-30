#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/market_data.hpp>

#include <functional>
#include <vector>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

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

private:
    client& owner_;
};

} // namespace binapi2::umf::rest
