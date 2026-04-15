// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API tab — step 0 placeholder.

#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace demo_ui {

using namespace ftxui;

Component make_ws_view(app_state& /*state*/, worker& /*wrk*/)
{
    return Renderer([] {
        return vbox({
                   text("WebSocket API") | bold,
                   separator(),
                   text("(step 0 — placeholder; wires up in step 5)"),
               })
               | flex;
    });
}

} // namespace demo_ui
