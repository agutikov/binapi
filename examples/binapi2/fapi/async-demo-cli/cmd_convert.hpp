// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_convert_quote(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_convert_accept(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_convert_order_status(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
