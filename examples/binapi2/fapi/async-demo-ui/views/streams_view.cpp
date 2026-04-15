// SPDX-License-Identifier: Apache-2.0
//
// Streams tab — step 0 placeholder.

#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace demo_ui {

using namespace ftxui;

Component make_streams_view(app_state& /*state*/, worker& /*wrk*/)
{
    return Renderer([] {
        return vbox({
                   text("Market Streams") | bold,
                   separator(),
                   text("(step 0 — placeholder; wires up in step 3+)"),
               })
               | flex;
    });
}

} // namespace demo_ui
