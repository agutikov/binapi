// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/market_data.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::fapi::rest {

result<std::vector<types::kline>>
market_data_service::klines(const types::kline_request& request)
{
    return owner_.execute<std::vector<types::kline>>(
        klines_endpoint.method, std::string{ klines_endpoint.path }, to_query_map(request), klines_endpoint.signed_request);
}

void
market_data_service::klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(klines(request)); });
}

result<std::vector<types::kline>>
market_data_service::mark_price_klines(const types::kline_request& request)
{
    return owner_.execute<std::vector<types::kline>>(mark_price_klines_endpoint.method,
                                                     std::string{ mark_price_klines_endpoint.path },
                                                     to_query_map(request),
                                                     mark_price_klines_endpoint.signed_request);
}

void
market_data_service::mark_price_klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(mark_price_klines(request)); });
}

result<std::vector<types::kline>>
market_data_service::premium_index_klines(const types::kline_request& request)
{
    return owner_.execute<std::vector<types::kline>>(premium_index_klines_endpoint.method,
                                                     std::string{ premium_index_klines_endpoint.path },
                                                     to_query_map(request),
                                                     premium_index_klines_endpoint.signed_request);
}

void
market_data_service::premium_index_klines(const types::kline_request& request,
                                          callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(premium_index_klines(request)); });
}

result<std::vector<types::book_ticker>>
market_data_service::book_tickers()
{
    return owner_.execute<std::vector<types::book_ticker>>(
        book_ticker_endpoint.method, std::string{ book_ticker_endpoint.path }, {}, book_ticker_endpoint.signed_request);
}

void
market_data_service::book_tickers(callback_type<std::vector<types::book_ticker>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(book_tickers()); });
}

result<std::vector<types::price_ticker>>
market_data_service::price_tickers()
{
    return owner_.execute<std::vector<types::price_ticker>>(
        price_ticker_endpoint.method, std::string{ price_ticker_endpoint.path }, {}, price_ticker_endpoint.signed_request);
}

void
market_data_service::price_tickers(callback_type<std::vector<types::price_ticker>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(price_tickers()); });
}

result<std::vector<types::price_ticker>>
market_data_service::price_tickers_v2()
{
    return owner_.execute<std::vector<types::price_ticker>>(price_ticker_v2_endpoint.method,
                                                                std::string{ price_ticker_v2_endpoint.path },
                                                                {},
                                                                price_ticker_v2_endpoint.signed_request);
}

void
market_data_service::price_tickers_v2(callback_type<std::vector<types::price_ticker>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(price_tickers_v2()); });
}

result<std::vector<types::ticker_24hr>>
market_data_service::ticker_24hrs()
{
    return owner_.execute<std::vector<types::ticker_24hr>>(
        ticker_24hr_endpoint.method, std::string{ ticker_24hr_endpoint.path }, {}, ticker_24hr_endpoint.signed_request);
}

void
market_data_service::ticker_24hrs(callback_type<std::vector<types::ticker_24hr>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(ticker_24hrs()); });
}

result<std::vector<types::mark_price>>
market_data_service::mark_prices()
{
    return owner_.execute<std::vector<types::mark_price>>(
        mark_price_endpoint.method, std::string{ mark_price_endpoint.path }, {}, mark_price_endpoint.signed_request);
}

void
market_data_service::mark_prices(callback_type<std::vector<types::mark_price>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(mark_prices()); });
}

result<std::vector<types::funding_rate_info>>
market_data_service::funding_rate_info()
{
    return owner_.execute<std::vector<types::funding_rate_info>>(funding_rate_info_endpoint.method,
                                                                 std::string{ funding_rate_info_endpoint.path },
                                                                 {},
                                                                 funding_rate_info_endpoint.signed_request);
}

void
market_data_service::funding_rate_info(callback_type<std::vector<types::funding_rate_info>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(funding_rate_info()); });
}

result<std::vector<types::open_interest_statistics_entry>>
market_data_service::open_interest_statistics(const types::futures_data_request& request)
{
    return owner_.execute<std::vector<types::open_interest_statistics_entry>>(
        open_interest_statistics_endpoint.method,
        std::string{ open_interest_statistics_endpoint.path },
        to_query_map(request),
        open_interest_statistics_endpoint.signed_request);
}

void
market_data_service::open_interest_statistics(const types::futures_data_request& request,
                                              callback_type<std::vector<types::open_interest_statistics_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(open_interest_statistics(request)); });
}

result<std::vector<types::long_short_ratio_entry>>
market_data_service::top_long_short_account_ratio(const types::futures_data_request& request)
{
    return owner_.execute<std::vector<types::long_short_ratio_entry>>(top_long_short_account_ratio_endpoint.method,
                                                                      std::string{ top_long_short_account_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      top_long_short_account_ratio_endpoint.signed_request);
}

void
market_data_service::top_long_short_account_ratio(const types::futures_data_request& request,
                                                  callback_type<std::vector<types::long_short_ratio_entry>> callback)
{
    detail::post_callback(owner_.context(), [this, request, callback = std::move(callback)]() mutable {
        callback(top_long_short_account_ratio(request));
    });
}

result<std::vector<types::long_short_ratio_entry>>
market_data_service::top_trader_long_short_ratio(const types::futures_data_request& request)
{
    return owner_.execute<std::vector<types::long_short_ratio_entry>>(top_trader_long_short_ratio_endpoint.method,
                                                                      std::string{ top_trader_long_short_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      top_trader_long_short_ratio_endpoint.signed_request);
}

void
market_data_service::top_trader_long_short_ratio(const types::futures_data_request& request,
                                                 callback_type<std::vector<types::long_short_ratio_entry>> callback)
{
    detail::post_callback(owner_.context(), [this, request, callback = std::move(callback)]() mutable {
        callback(top_trader_long_short_ratio(request));
    });
}

result<std::vector<types::long_short_ratio_entry>>
market_data_service::long_short_ratio(const types::futures_data_request& request)
{
    return owner_.execute<std::vector<types::long_short_ratio_entry>>(long_short_ratio_endpoint.method,
                                                                      std::string{ long_short_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      long_short_ratio_endpoint.signed_request);
}

void
market_data_service::long_short_ratio(const types::futures_data_request& request,
                                      callback_type<std::vector<types::long_short_ratio_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(long_short_ratio(request)); });
}

result<std::vector<types::taker_buy_sell_volume_entry>>
market_data_service::taker_buy_sell_volume(const types::futures_data_request& request)
{
    return owner_.execute<std::vector<types::taker_buy_sell_volume_entry>>(taker_buy_sell_volume_endpoint.method,
                                                                           std::string{ taker_buy_sell_volume_endpoint.path },
                                                                           to_query_map(request),
                                                                           taker_buy_sell_volume_endpoint.signed_request);
}

void
market_data_service::taker_buy_sell_volume(const types::futures_data_request& request,
                                           callback_type<std::vector<types::taker_buy_sell_volume_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(taker_buy_sell_volume(request)); });
}

} // namespace binapi2::fapi::rest
