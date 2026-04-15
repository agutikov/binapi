// SPDX-License-Identifier: Apache-2.0
//
// REST tab — step 0 placeholder.
//
// Step 1 will replace this with: command list (left) + params form (centre)
// + response pane with Raw / JSON / Tree tabs (right).

#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace demo_ui {

using namespace ftxui;

Component make_rest_view(app_state& /*state*/, worker& /*wrk*/)
{
    return Renderer([] {
        return vbox({
                   text("REST endpoints") | bold,
                   separator(),
                   text("(step 0 — placeholder; commands wire up in step 1+)"),
               })
               | flex;
    });
}

} // namespace demo_ui
