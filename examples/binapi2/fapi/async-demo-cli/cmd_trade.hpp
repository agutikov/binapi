// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

/// Register all Trade subcommands on `app`.
void register_cmd_trade(CLI::App& app, selected_cmd& sel);

} // namespace demo
