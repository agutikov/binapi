// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_account_info(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_balances(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_position_risk(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_income_history(binapi2::fapi::client& c, const args_t& args);

} // namespace demo
