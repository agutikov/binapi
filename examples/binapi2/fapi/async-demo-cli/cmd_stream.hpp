// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_stream_book_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_mark_price(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_kline(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_depth(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_all_book_tickers(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_all_tickers(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_all_mini_tickers(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_liquidation(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
