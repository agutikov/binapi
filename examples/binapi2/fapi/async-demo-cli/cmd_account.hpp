// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_account_info(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_balances(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_position_risk(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_income_history(binapi2::futures_usdm_api& c, const args_t& args);

boost::cobalt::task<int> cmd_account_config(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_symbol_config(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_multi_assets_mode(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_position_mode(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_rate_limit_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_leverage_bracket(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_commission_rate(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_bnb_burn(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_toggle_bnb_burn(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_quantitative_rules(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_pm_account_info(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_download_id_transaction(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_download_link_transaction(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_download_id_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_download_link_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_download_id_trade(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_download_link_trade(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
