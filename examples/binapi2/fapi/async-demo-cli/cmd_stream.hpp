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

boost::cobalt::task<int> cmd_stream_aggregate_trade(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_diff_depth(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_mini_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_all_liquidations(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_all_mark_prices(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_continuous_kline(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_composite_index(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_contract_info(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_asset_index(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_all_asset_index(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_trading_session(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_stream_rpi_diff_depth(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
