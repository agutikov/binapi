// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API command registry. Reuses the `rest::` types
// (`form_state`, `form_kind`, `cmd_ctx`, `rest_command`) so the tab-side
// view code for WS looks the same as REST.
//
// Difference from REST: each WS call creates a fresh WS client, connects,
// executes, and disconnects — there's no long-lived REST client to
// acquire. The session-logon command is a bespoke path (connect + logon,
// no execute) rather than going through `exec_ws_*`.

#pragma once

#include "../rest/commands.hpp"

#include <boost/cobalt/task.hpp>

#include <span>

namespace demo_ui::ws {

/// Flat list of all WS API commands (public + signed). Auth gating is
/// applied at the view layer via `rest::requires_auth(cmd.group)`.
std::span<const rest::rest_command> ws_commands();

/// Bespoke coroutine used by the `ws-logon` command. Public so the view
/// can spawn it without threading through the rest_command.run pointer.
boost::cobalt::task<void>
ws_logon_coro(worker& wrk,
              std::shared_ptr<capture_sink> sink,
              std::shared_ptr<request_capture> cap);

} // namespace demo_ui::ws
