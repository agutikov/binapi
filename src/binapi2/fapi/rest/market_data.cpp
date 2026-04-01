// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/market_data.hpp>

#include <binapi2/fapi/query.hpp>
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
    return owner_.execute<types::exchange_info_response>(exchange_info_endpoint.method,
                                                         std::string{ exchange_info_endpoint.path },
                                                         to_query_map(request),
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
    return owner_.execute<types::order_book_response>(
        order_book_endpoint.method, std::string{ order_book_endpoint.path }, to_query_map(request), order_book_endpoint.signed_request);
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
    return owner_.execute<std::vector<types::recent_trade>>(recent_trades_endpoint.method,
                                                            std::string{ recent_trades_endpoint.path },
                                                            to_query_map(request),
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
    return owner_.execute<std::vector<types::aggregate_trade>>(aggregate_trades_endpoint.method,
                                                               std::string{ aggregate_trades_endpoint.path },
                                                               to_query_map(request),
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
market_data_service::continuous_klines(const types::continuous_kline_request& request)
{
    return owner_.execute<std::vector<types::kline>>(continuous_klines_endpoint.method,
                                                     std::string{ continuous_klines_endpoint.path },
                                                     to_query_map(request),
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
    return owner_.execute<std::vector<types::kline>>(index_price_klines_endpoint.method,
                                                     std::string{ index_price_klines_endpoint.path },
                                                     to_query_map(request),
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

result<types::book_ticker>
market_data_service::book_ticker(const types::book_ticker_request& request)
{
    return owner_.execute<types::book_ticker>(
        book_ticker_endpoint.method, std::string{ book_ticker_endpoint.path }, to_query_map(request), book_ticker_endpoint.signed_request);
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
    return owner_.execute<types::price_ticker>(
        price_ticker_endpoint.method, std::string{ price_ticker_endpoint.path }, to_query_map(request), price_ticker_endpoint.signed_request);
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
    return owner_.execute<types::ticker_24hr>(
        ticker_24hr_endpoint.method, std::string{ ticker_24hr_endpoint.path }, to_query_map(request), ticker_24hr_endpoint.signed_request);
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
    return owner_.execute<types::mark_price>(
        mark_price_endpoint.method, std::string{ mark_price_endpoint.path }, to_query_map(request), mark_price_endpoint.signed_request);
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
    return owner_.execute<std::vector<types::funding_rate_history_entry>>(funding_rate_history_endpoint.method,
                                                                          std::string{ funding_rate_history_endpoint.path },
                                                                          to_query_map(request),
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
                                                to_query_map(request),
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

result<std::vector<types::recent_trade>>
market_data_service::historical_trades(const types::historical_trades_request& request)
{
    return owner_.execute<std::vector<types::recent_trade>>(historical_trades_endpoint.method,
                                                            std::string{ historical_trades_endpoint.path },
                                                            to_query_map(request),
                                                            historical_trades_endpoint.signed_request);
}

void
market_data_service::historical_trades(const types::historical_trades_request& request,
                                       callback_type<std::vector<types::recent_trade>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(historical_trades(request)); });
}

result<std::vector<types::basis_entry>>
market_data_service::basis(const types::basis_request& request)
{
    return owner_.execute<std::vector<types::basis_entry>>(
        basis_endpoint.method, std::string{ basis_endpoint.path }, to_query_map(request), basis_endpoint.signed_request);
}

void
market_data_service::basis(const types::basis_request& request, callback_type<std::vector<types::basis_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(basis(request)); });
}

result<types::price_ticker>
market_data_service::price_ticker_v2(const types::price_ticker_v2_request& request)
{
    return owner_.execute<types::price_ticker>(price_ticker_v2_endpoint.method,
                                                   std::string{ price_ticker_v2_endpoint.path },
                                                   to_query_map(request),
                                                   price_ticker_v2_endpoint.signed_request);
}

void
market_data_service::price_ticker_v2(const types::price_ticker_v2_request& request,
                                     callback_type<types::price_ticker> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(price_ticker_v2(request)); });
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

result<std::vector<types::delivery_price_entry>>
market_data_service::delivery_price(const types::delivery_price_request& request)
{
    return owner_.execute<std::vector<types::delivery_price_entry>>(delivery_price_endpoint.method,
                                                                     std::string{ delivery_price_endpoint.path },
                                                                     to_query_map(request),
                                                                     delivery_price_endpoint.signed_request);
}

void
market_data_service::delivery_price(const types::delivery_price_request& request,
                                    callback_type<std::vector<types::delivery_price_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(delivery_price(request)); });
}

result<std::vector<types::composite_index_info>>
market_data_service::composite_index_info(const types::composite_index_info_request& request)
{
    return owner_.execute<std::vector<types::composite_index_info>>(composite_index_info_endpoint.method,
                                                                     std::string{ composite_index_info_endpoint.path },
                                                                     to_query_map(request),
                                                                     composite_index_info_endpoint.signed_request);
}

void
market_data_service::composite_index_info(const types::composite_index_info_request& request,
                                          callback_type<std::vector<types::composite_index_info>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(composite_index_info(request)); });
}

result<types::index_constituents_response>
market_data_service::index_constituents(const types::index_constituents_request& request)
{
    return owner_.execute<types::index_constituents_response>(index_constituents_endpoint.method,
                                                              std::string{ index_constituents_endpoint.path },
                                                              to_query_map(request),
                                                              index_constituents_endpoint.signed_request);
}

void
market_data_service::index_constituents(const types::index_constituents_request& request,
                                        callback_type<types::index_constituents_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(index_constituents(request)); });
}

result<std::vector<types::asset_index>>
market_data_service::asset_index(const types::asset_index_request& request)
{
    return owner_.execute<std::vector<types::asset_index>>(asset_index_endpoint.method,
                                                                  std::string{ asset_index_endpoint.path },
                                                                  to_query_map(request),
                                                                  asset_index_endpoint.signed_request);
}

void
market_data_service::asset_index(const types::asset_index_request& request,
                                 callback_type<std::vector<types::asset_index>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(asset_index(request)); });
}

result<types::insurance_fund_response>
market_data_service::insurance_fund(const types::insurance_fund_request& request)
{
    return owner_.execute<types::insurance_fund_response>(insurance_fund_endpoint.method,
                                                                     std::string{ insurance_fund_endpoint.path },
                                                                     to_query_map(request),
                                                                     insurance_fund_endpoint.signed_request);
}

void
market_data_service::insurance_fund(const types::insurance_fund_request& request,
                                    callback_type<types::insurance_fund_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(insurance_fund(request)); });
}

result<std::vector<types::adl_risk_entry>>
market_data_service::adl_risk(const types::adl_risk_request& request)
{
    return owner_.execute<std::vector<types::adl_risk_entry>>(
        adl_risk_endpoint.method, std::string{ adl_risk_endpoint.path }, to_query_map(request), adl_risk_endpoint.signed_request);
}

void
market_data_service::adl_risk(const types::adl_risk_request& request, callback_type<std::vector<types::adl_risk_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(adl_risk(request)); });
}

result<types::order_book_response>
market_data_service::rpi_depth(const types::rpi_depth_request& request)
{
    return owner_.execute<types::order_book_response>(
        rpi_depth_endpoint.method, std::string{ rpi_depth_endpoint.path }, to_query_map(request), rpi_depth_endpoint.signed_request);
}

void
market_data_service::rpi_depth(const types::rpi_depth_request& request,
                               callback_type<types::order_book_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(rpi_depth(request)); });
}

result<types::trading_schedule_response>
market_data_service::trading_schedule(const types::trading_schedule_request&)
{
    query_map query;
    return owner_.execute<types::trading_schedule_response>(trading_schedule_endpoint.method,
                                                                      std::string{ trading_schedule_endpoint.path },
                                                                      query,
                                                                      trading_schedule_endpoint.signed_request);
}

void
market_data_service::trading_schedule(const types::trading_schedule_request& request,
                                      callback_type<types::trading_schedule_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(trading_schedule(request)); });
}

} // namespace binapi2::fapi::rest
