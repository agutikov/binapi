// SPDX-License-Identifier: Apache-2.0
//
// View factories — one per top-level tab. Step 0 returns placeholders so the
// layout renders end-to-end; later steps replace the bodies with real forms,
// response panes, stream views, etc.

#pragma once

#include "../app_state.hpp"
#include "../worker.hpp"

#include <ftxui/component/component.hpp>

namespace demo_ui {

ftxui::Component make_status_bar (app_state& state);
ftxui::Component make_rest_view  (app_state& state, worker& wrk);
ftxui::Component make_ws_view    (app_state& state, worker& wrk);
ftxui::Component make_streams_view(app_state& state, worker& wrk);
ftxui::Component make_orderbook_view(app_state& state, worker& wrk);

} // namespace demo_ui
