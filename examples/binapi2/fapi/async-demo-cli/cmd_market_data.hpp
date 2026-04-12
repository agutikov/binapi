// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_ping(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_time(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_exchange_info(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_order_book(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_recent_trades(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_book_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_book_tickers(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_price_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_price_tickers(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ticker_24hr(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_mark_price(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_mark_prices(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_klines(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_funding_rate(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_open_interest(binapi2::futures_usdm_api& c, const args_t& args);

boost::cobalt::task<int> cmd_aggregate_trades(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_historical_trades(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_continuous_kline(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_index_price_kline(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_mark_price_klines(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_premium_index_klines(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_price_ticker_v2(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_price_tickers_v2(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ticker_24hrs(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_funding_rate_info(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_open_interest_stats(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_top_ls_account_ratio(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_top_ls_trader_ratio(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_long_short_ratio(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_taker_volume(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_basis(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_delivery_price(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_composite_index_info(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_index_constituents(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_asset_index(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_insurance_fund(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_adl_risk(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_rpi_depth(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_trading_schedule(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
