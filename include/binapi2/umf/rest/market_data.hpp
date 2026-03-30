#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/market_data.hpp>

#include <vector>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

class market_data_service
{
public:
    explicit market_data_service(client& owner) noexcept;

    [[nodiscard]] result<types::empty_response> ping();
    [[nodiscard]] result<types::server_time_response> server_time();
    [[nodiscard]] result<types::exchange_info_response> exchange_info(const types::exchange_info_request& request = {});
    [[nodiscard]] result<types::order_book_response> order_book(const types::order_book_request& request);
    [[nodiscard]] result<std::vector<types::recent_trade>> recent_trades(const types::recent_trades_request& request);
    [[nodiscard]] result<std::vector<types::aggregate_trade>> aggregate_trades(const types::aggregate_trades_request& request);
    [[nodiscard]] result<std::vector<types::kline>> klines(const types::kline_request& request);
    [[nodiscard]] result<std::vector<types::kline>> continuous_klines(const types::continuous_kline_request& request);
    [[nodiscard]] result<std::vector<types::kline>> index_price_klines(const types::index_price_kline_request& request);
    [[nodiscard]] result<std::vector<types::kline>> mark_price_klines(const types::kline_request& request);
    [[nodiscard]] result<std::vector<types::kline>> premium_index_klines(const types::kline_request& request);
    [[nodiscard]] result<types::book_ticker> book_ticker(const types::book_ticker_request& request);
    [[nodiscard]] result<std::vector<types::book_ticker>> book_tickers();
    [[nodiscard]] result<types::price_ticker> price_ticker(const types::price_ticker_request& request);
    [[nodiscard]] result<std::vector<types::price_ticker>> price_tickers();
    [[nodiscard]] result<types::ticker_24hr> ticker_24hr(const types::ticker_24hr_request& request);
    [[nodiscard]] result<std::vector<types::ticker_24hr>> ticker_24hrs();
    [[nodiscard]] result<types::mark_price> mark_price(const types::mark_price_request& request);
    [[nodiscard]] result<std::vector<types::mark_price>> mark_prices();
    [[nodiscard]] result<std::vector<types::funding_rate_history_entry>> funding_rate_history(
        const types::funding_rate_history_request& request = {});
    [[nodiscard]] result<std::vector<types::funding_rate_info>> funding_rate_info();
    [[nodiscard]] result<types::open_interest> open_interest(const types::open_interest_request& request);
    [[nodiscard]] result<std::vector<types::open_interest_statistics_entry>> open_interest_statistics(
        const types::futures_data_request& request);
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> top_long_short_account_ratio(
        const types::futures_data_request& request);
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> top_trader_long_short_ratio(
        const types::futures_data_request& request);
    [[nodiscard]] result<std::vector<types::long_short_ratio_entry>> long_short_ratio(
        const types::futures_data_request& request);
    [[nodiscard]] result<std::vector<types::taker_buy_sell_volume_entry>> taker_buy_sell_volume(
        const types::futures_data_request& request);
    [[nodiscard]] result<std::vector<types::recent_trade>> historical_trades(const types::historical_trades_request& request);

private:
    client& owner_;
};

} // namespace binapi2::umf::rest
