// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

/// Register Pipeline Order Book subcommands on `app`.
void register_cmd_pipeline_order_book(CLI::App& app, selected_cmd& sel);

} // namespace demo
