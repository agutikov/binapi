// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_new_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_test_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_cancel_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_query_order(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_open_orders(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
