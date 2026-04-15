// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

/// Register all Market Data subcommands on `app`.
/// Each subcommand's callback stashes a factory in `sel`.
void register_cmd_market_data(CLI::App& app, selected_cmd& sel);

} // namespace demo
