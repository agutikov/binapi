// SPDX-License-Identifier: Apache-2.0
//
// Top status bar. Reads atomics + a short string under mutex.
// Style: [TESTNET|LIVE]  creds: …  jobs: N  | <status_message>

#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>

namespace demo_ui {

using namespace ftxui;

Component make_status_bar(app_state& state)
{
    return Renderer([&state] {
        const char* env_label = state.use_testnet ? "TESTNET" : "LIVE";
        const Color env_color = state.use_testnet ? Color::Yellow : Color::Red;

        Element creds;
        if (state.credentials_failed) {
            std::string err;
            {
                std::lock_guard lk(state.mtx);
                err = state.credentials_error;
            }
            creds = text("creds: failed (" + err + ")") | color(Color::Red);
        } else if (state.credentials_loaded) {
            creds = text("creds: ok") | color(Color::Green);
        } else {
            creds = text("creds: loading…") | color(Color::GrayDark);
        }

        std::string status_copy;
        {
            std::lock_guard lk(state.mtx);
            status_copy = state.status_message;
        }

        return hbox({
                   text(" ") | bgcolor(env_color),
                   text(std::string(" ") + env_label + " ") | bgcolor(env_color)
                       | color(Color::Black) | bold,
                   text("  "),
                   creds,
                   text("  "),
                   text("jobs: " + std::to_string(state.active_jobs.load())),
                   text("  │  "),
                   text(status_copy) | dim,
                   filler(),
               })
               | bgcolor(Color::GrayDark) | color(Color::White);
    });
}

} // namespace demo_ui
