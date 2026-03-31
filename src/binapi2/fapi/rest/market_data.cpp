// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/market_data.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::fapi::rest {

market_data_service::market_data_service(binapi2::fapi::client& owner) noexcept : owner_(owner) {}

result<types::empty_response>
market_data_service::ping()
{
    return owner_.execute<types::empty_response>(
        ping_endpoint.method, std::string{ ping_endpoint.path }, {}, ping_endpoint.signed_request);
}

void
market_data_service::ping(callback_type<types::empty_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(ping()); });
}

result<types::server_time_response>
market_data_service::server_time()
{
    return owner_.execute<types::server_time_response>(
        server_time_endpoint.method, std::string{ server_time_endpoint.path }, {}, server_time_endpoint.signed_request);
}

void
market_data_service::server_time(callback_type<types::server_time_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(server_time()); });
}

result<types::exchange_info_response>
market_data_service::exchange_info(const types::exchange_info_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::exchange_info_response>(exchange_info_endpoint.method,
                                                         std::string{ exchange_info_endpoint.path },
                                                         query,
                                                         exchange_info_endpoint.signed_request);
}

void
market_data_service::exchange_info(const types::exchange_info_request& request,
                                   callback_type<types::exchange_info_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(exchange_info(request)); });
}

result<types::order_book_response>
market_data_service::order_book(const types::order_book_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<types::order_book_response>(
        order_book_endpoint.method, std::string{ order_book_endpoint.path }, query, order_book_endpoint.signed_request);
}

void
market_data_service::order_book(const types::order_book_request& request, callback_type<types::order_book_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(order_book(request)); });
}

result<std::vector<types::recent_trade>>
market_data_service::recent_trades(const types::recent_trades_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::recent_trade>>(recent_trades_endpoint.method,
                                                            std::string{ recent_trades_endpoint.path },
                                                            query,
                                                            recent_trades_endpoint.signed_request);
}

void
market_data_service::recent_trades(const types::recent_trades_request& request,
                                   callback_type<std::vector<types::recent_trade>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(recent_trades(request)); });
}

result<std::vector<types::aggregate_trade>>
market_data_service::aggregate_trades(const types::aggregate_trades_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.fromId) {
        query["fromId"] = std::to_string(*request.fromId);
    }
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::aggregate_trade>>(aggregate_trades_endpoint.method,
                                                               std::string{ aggregate_trades_endpoint.path },
                                                               query,
                                                               aggregate_trades_endpoint.signed_request);
}

void
market_data_service::aggregate_trades(const types::aggregate_trades_request& request,
                                      callback_type<std::vector<types::aggregate_trade>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(aggregate_trades(request)); });
}

result<std::vector<types::kline>>
market_data_service::klines(const types::kline_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "interval", to_string(request.interval) } };
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(
        klines_endpoint.method, std::string{ klines_endpoint.path }, query, klines_endpoint.signed_request);
}

void
market_data_service::klines(const types::kline_request& request, callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(klines(request)); });
}

result<std::vector<types::kline>>
market_data_service::continuous_klines(const types::continuous_kline_request& request)
{
    query_map query{ { "pair", request.pair },
                     { "contractType", to_string(request.contractType) },
                     { "interval", to_string(request.interval) } };
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(continuous_klines_endpoint.method,
                                                     std::string{ continuous_klines_endpoint.path },
                                                     query,
                                                     continuous_klines_endpoint.signed_request);
}

void
market_data_service::continuous_klines(const types::continuous_kline_request& request,
                                       callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(continuous_klines(request)); });
}

result<std::vector<types::kline>>
market_data_service::index_price_klines(const types::index_price_kline_request& request)
{
    query_map query{ { "pair", request.pair }, { "interval", to_string(request.interval) } };
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(index_price_klines_endpoint.method,
                                                     std::string{ index_price_klines_endpoint.path },
                                                     query,
                                                     index_price_klines_endpoint.signed_request);
}

void
market_data_service::index_price_klines(const types::index_price_kline_request& request,
                                        callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(index_price_klines(request)); });
}

result<std::vector<types::kline>>
market_data_service::mark_price_klines(const types::kline_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "interval", to_string(request.interval) } };
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(mark_price_klines_endpoint.method,
                                                     std::string{ mark_price_klines_endpoint.path },
                                                     query,
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
    query_map query{ { "symbol", request.symbol }, { "interval", to_string(request.interval) } };
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(premium_index_klines_endpoint.method,
                                                     std::string{ premium_index_klines_endpoint.path },
                                                     query,
                                                     premium_index_klines_endpoint.signed_request);
}

void
market_data_service::premium_index_klines(const types::kline_request& request,
                                          callback_type<std::vector<types::kline>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(premium_index_klines(request)); });
}

result<types::book_ticker>
market_data_service::book_ticker(const types::book_ticker_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::book_ticker>(
        book_ticker_endpoint.method, std::string{ book_ticker_endpoint.path }, query, book_ticker_endpoint.signed_request);
}

void
market_data_service::book_ticker(const types::book_ticker_request& request, callback_type<types::book_ticker> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(book_ticker(request)); });
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

result<types::price_ticker>
market_data_service::price_ticker(const types::price_ticker_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::price_ticker>(
        price_ticker_endpoint.method, std::string{ price_ticker_endpoint.path }, query, price_ticker_endpoint.signed_request);
}

void
market_data_service::price_ticker(const types::price_ticker_request& request, callback_type<types::price_ticker> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(price_ticker(request)); });
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

result<types::ticker_24hr>
market_data_service::ticker_24hr(const types::ticker_24hr_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::ticker_24hr>(
        ticker_24hr_endpoint.method, std::string{ ticker_24hr_endpoint.path }, query, ticker_24hr_endpoint.signed_request);
}

void
market_data_service::ticker_24hr(const types::ticker_24hr_request& request, callback_type<types::ticker_24hr> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(ticker_24hr(request)); });
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

result<types::mark_price>
market_data_service::mark_price(const types::mark_price_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::mark_price>(
        mark_price_endpoint.method, std::string{ mark_price_endpoint.path }, query, mark_price_endpoint.signed_request);
}

void
market_data_service::mark_price(const types::mark_price_request& request, callback_type<types::mark_price> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(mark_price(request)); });
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

result<std::vector<types::funding_rate_history_entry>>
market_data_service::funding_rate_history(const types::funding_rate_history_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::funding_rate_history_entry>>(funding_rate_history_endpoint.method,
                                                                          std::string{ funding_rate_history_endpoint.path },
                                                                          query,
                                                                          funding_rate_history_endpoint.signed_request);
}

void
market_data_service::funding_rate_history(const types::funding_rate_history_request& request,
                                          callback_type<std::vector<types::funding_rate_history_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(funding_rate_history(request)); });
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

result<types::open_interest>
market_data_service::open_interest(const types::open_interest_request& request)
{
    return owner_.execute<types::open_interest>(open_interest_endpoint.method,
                                                std::string{ open_interest_endpoint.path },
                                                { { "symbol", request.symbol } },
                                                open_interest_endpoint.signed_request);
}

void
market_data_service::open_interest(const types::open_interest_request& request, callback_type<types::open_interest> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(open_interest(request)); });
}

result<std::vector<types::open_interest_statistics_entry>>
market_data_service::open_interest_statistics(const types::futures_data_request& request)
{
    return owner_.execute<std::vector<types::open_interest_statistics_entry>>(
        open_interest_statistics_endpoint.method,
        std::string{ open_interest_statistics_endpoint.path },
        detail::make_futures_data_query(request),
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
                                                                      detail::make_futures_data_query(request),
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
                                                                      detail::make_futures_data_query(request),
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
                                                                      detail::make_futures_data_query(request),
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
                                                                           detail::make_futures_data_query(request),
                                                                           taker_buy_sell_volume_endpoint.signed_request);
}

void
market_data_service::taker_buy_sell_volume(const types::futures_data_request& request,
                                           callback_type<std::vector<types::taker_buy_sell_volume_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(taker_buy_sell_volume(request)); });
}

result<std::vector<types::recent_trade>>
market_data_service::historical_trades(const types::historical_trades_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    if (request.fromId) {
        query["fromId"] = std::to_string(*request.fromId);
    }
    return owner_.execute<std::vector<types::recent_trade>>(historical_trades_endpoint.method,
                                                            std::string{ historical_trades_endpoint.path },
                                                            query,
                                                            historical_trades_endpoint.signed_request);
}

void
market_data_service::historical_trades(const types::historical_trades_request& request,
                                       callback_type<std::vector<types::recent_trade>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(historical_trades(request)); });
}

} // namespace binapi2::fapi::rest
