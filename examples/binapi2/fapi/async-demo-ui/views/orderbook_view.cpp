// SPDX-License-Identifier: Apache-2.0
//
// Order Book tab — step 0 placeholder.

#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace demo_ui {

using namespace ftxui;

Component make_orderbook_view(app_state& /*state*/, worker& /*wrk*/)
{
    return Renderer([] {
        return vbox({
                   text("Local Order Book") | bold,
                   separator(),
                   text("(step 0 — placeholder; wires up in step 2 / 7)"),
               })
               | flex;
    });
}

} // namespace demo_ui
