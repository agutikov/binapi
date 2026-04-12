// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_new_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_test_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_cancel_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_query_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_open_orders(binapi2::futures_usdm_api& c, const args_t& args);

boost::cobalt::task<int> cmd_modify_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_cancel_multiple_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_cancel_all_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_auto_cancel(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_query_open_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_all_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_position_info_v3(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_adl_quantile(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_force_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_account_trades(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_change_position_mode(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_change_multi_assets_mode(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_change_leverage(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_change_margin_type(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_modify_isolated_margin(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_position_margin_history(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_order_modify_history(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_new_algo_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_cancel_algo_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_query_algo_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_all_algo_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_open_algo_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_cancel_all_algo_orders(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_tradfi_perps(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
