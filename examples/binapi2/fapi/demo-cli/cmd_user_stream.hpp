// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

int cmd_listen_key_start(const args_t& args);
int cmd_listen_key_keepalive(const args_t& args);
int cmd_listen_key_close(const args_t& args);
int cmd_user_stream(const args_t& args);

} // namespace demo
