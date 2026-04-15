// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

/// Register all User Data Streams subcommands on `app`.
void register_cmd_user_stream(CLI::App& app, selected_cmd& sel);

} // namespace demo
