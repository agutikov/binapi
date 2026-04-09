// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_ws_logon(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_account_status(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_order_place(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_ws_order_cancel(binapi2::fapi::client& c, const args_t& args);

} // namespace demo
