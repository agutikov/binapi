// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

/// Register all WebSocket API subcommands on `app`.
void register_cmd_ws_api(CLI::App& app, selected_cmd& sel);

} // namespace demo
