// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements market data REST endpoints that are not fully handled by
/// the generated_endpoints machinery. Methods here fall into two categories:
/// (1) endpoints whose request/response types are shared across multiple
/// paths (e.g. klines, mark_price_klines, premium_index_klines all use
/// kline_request_t -> vector<kline_t>), and (2) parameterless endpoints that
/// return aggregate collections (book_tickers, price_tickers, etc.) and need
/// no query serialization. All async variants delegate to their sync
/// counterparts.

#include <binapi2/fapi/rest/market_data.hpp>

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

result<std::vector<types::kline_t>>
market_data_service::klines(const types::kline_request_t& request)
{
    return owner_.execute<std::vector<types::kline_t>>(
        klines_endpoint.method, std::string{ klines_endpoint.path }, to_query_map(request), klines_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::kline_t>>>
market_data_service::async_klines(const types::kline_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::kline_t>>(
        klines_endpoint.method, std::string{ klines_endpoint.path }, to_query_map(request), klines_endpoint.signed_request);
}

result<std::vector<types::kline_t>>
market_data_service::mark_price_klines(const types::kline_request_t& request)
{
    return owner_.execute<std::vector<types::kline_t>>(mark_price_klines_endpoint.method,
                                                     std::string{ mark_price_klines_endpoint.path },
                                                     to_query_map(request),
                                                     mark_price_klines_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::kline_t>>>
market_data_service::async_mark_price_klines(const types::kline_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::kline_t>>(mark_price_klines_endpoint.method,
                                                     std::string{ mark_price_klines_endpoint.path },
                                                     to_query_map(request),
                                                     mark_price_klines_endpoint.signed_request);
}

result<std::vector<types::kline_t>>
market_data_service::premium_index_klines(const types::kline_request_t& request)
{
    return owner_.execute<std::vector<types::kline_t>>(premium_index_klines_endpoint.method,
                                                     std::string{ premium_index_klines_endpoint.path },
                                                     to_query_map(request),
                                                     premium_index_klines_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::kline_t>>>
market_data_service::async_premium_index_klines(const types::kline_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::kline_t>>(premium_index_klines_endpoint.method,
                                                     std::string{ premium_index_klines_endpoint.path },
                                                     to_query_map(request),
                                                     premium_index_klines_endpoint.signed_request);
}

result<std::vector<types::book_ticker_t>>
market_data_service::book_tickers()
{
    return owner_.execute<std::vector<types::book_ticker_t>>(
        book_ticker_endpoint.method, std::string{ book_ticker_endpoint.path }, {}, book_ticker_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::book_ticker_t>>>
market_data_service::async_book_tickers()
{
    co_return co_await owner_.async_execute<std::vector<types::book_ticker_t>>(
        book_ticker_endpoint.method, std::string{ book_ticker_endpoint.path }, {}, book_ticker_endpoint.signed_request);
}

result<std::vector<types::price_ticker_t>>
market_data_service::price_tickers()
{
    return owner_.execute<std::vector<types::price_ticker_t>>(
        price_ticker_endpoint.method, std::string{ price_ticker_endpoint.path }, {}, price_ticker_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::price_ticker_t>>>
market_data_service::async_price_tickers()
{
    co_return co_await owner_.async_execute<std::vector<types::price_ticker_t>>(
        price_ticker_endpoint.method, std::string{ price_ticker_endpoint.path }, {}, price_ticker_endpoint.signed_request);
}

result<std::vector<types::price_ticker_t>>
market_data_service::price_tickers_v2()
{
    return owner_.execute<std::vector<types::price_ticker_t>>(price_ticker_v2_endpoint.method,
                                                                std::string{ price_ticker_v2_endpoint.path },
                                                                {},
                                                                price_ticker_v2_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::price_ticker_t>>>
market_data_service::async_price_tickers_v2()
{
    co_return co_await owner_.async_execute<std::vector<types::price_ticker_t>>(price_ticker_v2_endpoint.method,
                                                                std::string{ price_ticker_v2_endpoint.path },
                                                                {},
                                                                price_ticker_v2_endpoint.signed_request);
}

result<std::vector<types::ticker_24hr_t>>
market_data_service::ticker_24hrs()
{
    return owner_.execute<std::vector<types::ticker_24hr_t>>(
        ticker_24hr_endpoint.method, std::string{ ticker_24hr_endpoint.path }, {}, ticker_24hr_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::ticker_24hr_t>>>
market_data_service::async_ticker_24hrs()
{
    co_return co_await owner_.async_execute<std::vector<types::ticker_24hr_t>>(
        ticker_24hr_endpoint.method, std::string{ ticker_24hr_endpoint.path }, {}, ticker_24hr_endpoint.signed_request);
}

result<std::vector<types::mark_price_t>>
market_data_service::mark_prices()
{
    return owner_.execute<std::vector<types::mark_price_t>>(
        mark_price_endpoint.method, std::string{ mark_price_endpoint.path }, {}, mark_price_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::mark_price_t>>>
market_data_service::async_mark_prices()
{
    co_return co_await owner_.async_execute<std::vector<types::mark_price_t>>(
        mark_price_endpoint.method, std::string{ mark_price_endpoint.path }, {}, mark_price_endpoint.signed_request);
}

result<std::vector<types::funding_rate_info_t>>
market_data_service::funding_rate_info_t()
{
    return owner_.execute<std::vector<types::funding_rate_info_t>>(funding_rate_info_endpoint.method,
                                                                 std::string{ funding_rate_info_endpoint.path },
                                                                 {},
                                                                 funding_rate_info_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::funding_rate_info_t>>>
market_data_service::async_funding_rate_info()
{
    co_return co_await owner_.async_execute<std::vector<types::funding_rate_info_t>>(funding_rate_info_endpoint.method,
                                                                 std::string{ funding_rate_info_endpoint.path },
                                                                 {},
                                                                 funding_rate_info_endpoint.signed_request);
}

result<std::vector<types::open_interest_statistics_entry_t>>
market_data_service::open_interest_statistics(const types::futures_data_request_t& request)
{
    return owner_.execute<std::vector<types::open_interest_statistics_entry_t>>(
        open_interest_statistics_endpoint.method,
        std::string{ open_interest_statistics_endpoint.path },
        to_query_map(request),
        open_interest_statistics_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::open_interest_statistics_entry_t>>>
market_data_service::async_open_interest_statistics(const types::futures_data_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::open_interest_statistics_entry_t>>(
        open_interest_statistics_endpoint.method,
        std::string{ open_interest_statistics_endpoint.path },
        to_query_map(request),
        open_interest_statistics_endpoint.signed_request);
}

result<std::vector<types::long_short_ratio_entry_t>>
market_data_service::top_long_short_account_ratio(const types::futures_data_request_t& request)
{
    return owner_.execute<std::vector<types::long_short_ratio_entry_t>>(top_long_short_account_ratio_endpoint.method,
                                                                      std::string{ top_long_short_account_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      top_long_short_account_ratio_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::long_short_ratio_entry_t>>>
market_data_service::async_top_long_short_account_ratio(const types::futures_data_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::long_short_ratio_entry_t>>(top_long_short_account_ratio_endpoint.method,
                                                                      std::string{ top_long_short_account_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      top_long_short_account_ratio_endpoint.signed_request);
}

result<std::vector<types::long_short_ratio_entry_t>>
market_data_service::top_trader_long_short_ratio(const types::futures_data_request_t& request)
{
    return owner_.execute<std::vector<types::long_short_ratio_entry_t>>(top_trader_long_short_ratio_endpoint.method,
                                                                      std::string{ top_trader_long_short_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      top_trader_long_short_ratio_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::long_short_ratio_entry_t>>>
market_data_service::async_top_trader_long_short_ratio(const types::futures_data_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::long_short_ratio_entry_t>>(top_trader_long_short_ratio_endpoint.method,
                                                                      std::string{ top_trader_long_short_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      top_trader_long_short_ratio_endpoint.signed_request);
}

result<std::vector<types::long_short_ratio_entry_t>>
market_data_service::long_short_ratio(const types::futures_data_request_t& request)
{
    return owner_.execute<std::vector<types::long_short_ratio_entry_t>>(long_short_ratio_endpoint.method,
                                                                      std::string{ long_short_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      long_short_ratio_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::long_short_ratio_entry_t>>>
market_data_service::async_long_short_ratio(const types::futures_data_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::long_short_ratio_entry_t>>(long_short_ratio_endpoint.method,
                                                                      std::string{ long_short_ratio_endpoint.path },
                                                                      to_query_map(request),
                                                                      long_short_ratio_endpoint.signed_request);
}

result<std::vector<types::taker_buy_sell_volume_entry_t>>
market_data_service::taker_buy_sell_volume(const types::futures_data_request_t& request)
{
    return owner_.execute<std::vector<types::taker_buy_sell_volume_entry_t>>(taker_buy_sell_volume_endpoint.method,
                                                                           std::string{ taker_buy_sell_volume_endpoint.path },
                                                                           to_query_map(request),
                                                                           taker_buy_sell_volume_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::taker_buy_sell_volume_entry_t>>>
market_data_service::async_taker_buy_sell_volume(const types::futures_data_request_t& request)
{
    co_return co_await owner_.async_execute<std::vector<types::taker_buy_sell_volume_entry_t>>(taker_buy_sell_volume_endpoint.method,
                                                                           std::string{ taker_buy_sell_volume_endpoint.path },
                                                                           to_query_map(request),
                                                                           taker_buy_sell_volume_endpoint.signed_request);
}

} // namespace binapi2::fapi::rest
