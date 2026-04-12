// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_ws_logon(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_account_status(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_order_place(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_order_cancel(binapi2::futures_usdm_api& c, const args_t& args);

boost::cobalt::task<int> cmd_ws_book_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_price_ticker(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_order_query(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_order_modify(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_position(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_account_status_v2(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_account_balance(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_algo_order_place(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_algo_order_cancel(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_user_stream_start(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_user_stream_ping(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_user_stream_stop(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
