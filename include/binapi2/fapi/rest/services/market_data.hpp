// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Market data REST service — public endpoints, no authentication.

#pragma once

#include <binapi2/fapi/rest/services/service.hpp>
#include <binapi2/fapi/types/market_data.hpp>

namespace binapi2::fapi::rest {

class market_data_service : public service
{
public:
    using service::service;

    template<class Request>
        requires is_market_data_request<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>
    {
        co_return co_await pipeline_.async_execute(request);
    }

    using ping_request = types::ping_request_t;
    using server_time_request = types::server_time_request_t;
    using exchange_info_request = types::exchange_info_request_t;
    using order_book_request = types::order_book_request_t;
    using recent_trades_request = types::recent_trades_request_t;
    using aggregate_trades_request = types::aggregate_trades_request_t;
    using historical_trades_request = types::historical_trades_request_t;
    using klines_request = types::klines_request_t;
    using mark_price_klines_request = types::mark_price_klines_request_t;
    using premium_index_klines_request = types::premium_index_klines_request_t;
    using continuous_kline_request = types::continuous_kline_request_t;
    using index_price_kline_request = types::index_price_kline_request_t;
    using book_ticker_request = types::book_ticker_request_t;
    using book_tickers_request = types::book_tickers_request_t;
    using price_ticker_request = types::price_ticker_request_t;
    using price_tickers_request = types::price_tickers_request_t;
    using price_tickers_v2_request = types::price_tickers_v2_request_t;
    using ticker_24hr_request = types::ticker_24hr_request_t;
    using ticker_24hrs_request = types::ticker_24hrs_request_t;
    using mark_price_request = types::mark_price_request_t;
    using mark_prices_request = types::mark_prices_request_t;
    using funding_rate_history_request = types::funding_rate_history_request_t;
    using funding_rate_info_request = types::funding_rate_info_request_t;
    using open_interest_request = types::open_interest_request_t;
    using open_interest_statistics_request = types::open_interest_statistics_request_t;
    using top_long_short_account_ratio_request = types::top_long_short_account_ratio_request_t;
    using top_trader_long_short_ratio_request = types::top_trader_long_short_ratio_request_t;
    using long_short_ratio_request = types::long_short_ratio_request_t;
    using taker_buy_sell_volume_request = types::taker_buy_sell_volume_request_t;
    using basis_request = types::basis_request_t;
    using delivery_price_request = types::delivery_price_request_t;
    using composite_index_info_request = types::composite_index_info_request_t;
    using index_constituents_request = types::index_constituents_request_t;
    using asset_index_request = types::asset_index_request_t;
    using insurance_fund_request = types::insurance_fund_request_t;
    using insurance_funds_request = types::insurance_funds_request_t;
    using adl_risk_request = types::adl_risk_request_t;
    using rpi_depth_request = types::rpi_depth_request_t;
    using trading_schedule_request = types::trading_schedule_request_t;
};

} // namespace binapi2::fapi::rest
