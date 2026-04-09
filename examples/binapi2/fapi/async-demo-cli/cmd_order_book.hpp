// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_order_book_live(binapi2::fapi::client& c, const args_t& args);

} // namespace demo
