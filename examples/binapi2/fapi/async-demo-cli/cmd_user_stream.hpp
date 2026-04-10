// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

boost::cobalt::task<int> cmd_listen_key_start(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_listen_key_keepalive(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_listen_key_close(binapi2::futures_usdm_api& c, const args_t& args);
boost::cobalt::task<int> cmd_user_stream(binapi2::futures_usdm_api& c, const args_t& args);

} // namespace demo
