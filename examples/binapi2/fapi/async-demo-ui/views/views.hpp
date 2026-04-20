// SPDX-License-Identifier: Apache-2.0
//
// View factories — one per top-level tab. Each returns a `tab_view`: the
// tab Component plus a `hints()` fn that main.cpp calls at render time to
// populate the shared bottom keybar.

#pragma once

#include "../app_state.hpp"
#include "../worker.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>
#include <vector>

namespace demo_ui {

/// One tab's renderable + its context-aware keybar hints.
///
/// `hints` is called by the outer layout every render, so it can probe
/// the tab's focused zone and return different chips accordingly.
struct tab_view
{
    ftxui::Component component;
    std::function<std::vector<ftxui::Element>()> hints;
};

/// A single bottom-bar chip: a highlighted key glyph + a one-word
/// description. Use this so every tab's hints look the same.
inline ftxui::Element key_chip(const char* keys, const char* desc)
{
    using namespace ftxui;
    return hbox({ text(keys) | bold | color(Color::Yellow),
                  text(" "), text(desc) });
}

ftxui::Component make_status_bar   (app_state& state);
tab_view         make_rest_view    (app_state& state, worker& wrk);
tab_view         make_ws_view      (app_state& state, worker& wrk);
tab_view         make_streams_view (app_state& state, worker& wrk);
tab_view         make_orderbook_view(app_state& state, worker& wrk);

} // namespace demo_ui
