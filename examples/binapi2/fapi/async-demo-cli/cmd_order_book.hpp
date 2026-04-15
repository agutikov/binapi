// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

/// Register Order Book subcommands on `app`.
void register_cmd_order_book(CLI::App& app, selected_cmd& sel);

} // namespace demo
