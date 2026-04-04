// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

int cmd_new_order(const args_t& args);
int cmd_test_order(const args_t& args);
int cmd_cancel_order(const args_t& args);
int cmd_query_order(const args_t& args);
int cmd_open_orders(const args_t& args);

} // namespace demo
