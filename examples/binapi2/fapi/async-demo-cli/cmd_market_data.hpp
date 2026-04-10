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

} // namespace demo
