// SPDX-License-Identifier: Apache-2.0
//
// Streams tab.
//
// Same shape as REST / WS tabs: scrollable menu on the left, a vertical
// form container in the middle (symbol / pair / interval / levels /
// speed, each `Maybe`-gated per form_kind), and a virtualized event
// list on the right that scrolls the pretty-printed event ring.
//
// The event list uses the same `scroll_model` + `virtual_scroll_render`
// helpers as the REST response panes, so PageUp/Down, mouse wheel, and
// the custom scrollbar behave identically.
//
// Start / Stop is bound to Enter on the menu: first Enter starts the
// subscription, second Enter toggles `stream_capture::stop`, which the
// generator loop in `lib::exec_stream` observes between events.

#include "../app_state.hpp"
#include "../streams/commands.hpp"
#include "../streams/stream_capture.hpp"
#include "../util/virtual_scroll.hpp"
#include "../util/wrap_text.hpp"
#include "../worker.hpp"
#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>

#include <spdlog/spdlog.h>

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;

namespace {

namespace S = demo_ui::streams;

} // namespace

tab_view make_streams_view(app_state& state, worker& wrk)
{
    spdlog::debug("make_streams_view: entering");

    auto cmds = S::stream_commands();

    auto titles = std::make_shared<std::vector<std::string>>();
    titles->reserve(cmds.size());
    for (const auto& c : cmds) titles->emplace_back(c.name);

    // Per-stream capture so switching in the menu preserves each
    // stream's buffer and counters.
    auto caps = std::make_shared<std::vector<std::shared_ptr<stream_capture>>>();
    caps->reserve(cmds.size());
    for (std::size_t i = 0; i < cmds.size(); ++i)
        caps->emplace_back(std::make_shared<stream_capture>());

    // Per-stream virtualized scroll state + cached wrapped rows. Each
    // stream's event list scrolls independently.
    auto models = std::make_shared<std::vector<std::shared_ptr<scroll_model>>>();
    auto cached_rows = std::make_shared<std::vector<std::vector<Element>>>();
    auto cached_stamps = std::make_shared<std::vector<std::uint64_t>>();
    auto cached_widths = std::make_shared<std::vector<int>>();
    models->reserve(cmds.size());
    cached_rows->resize(cmds.size());
    cached_stamps->assign(cmds.size(), std::uint64_t{ 0 });
    cached_widths->assign(cmds.size(), -1);
    for (std::size_t i = 0; i < cmds.size(); ++i)
        models->emplace_back(std::make_shared<scroll_model>());

    auto menu_index = std::make_shared<int>(0);
    auto form       = std::make_shared<S::form_state>();
    auto event_box  = std::make_shared<Box>();

    auto selected_cmd = [cmds, menu_index]() -> const S::stream_command* {
        auto idx = static_cast<std::size_t>(*menu_index);
        if (idx >= cmds.size()) return nullptr;
        return &cmds[idx];
    };

    auto selected_form_kind = [selected_cmd]() -> S::form_kind {
        if (auto* c = selected_cmd()) return c->form;
        return S::form_kind::no_args;
    };

    auto cmd_menu = Menu(titles.get(), menu_index.get());
    auto cmd_menu_with_enter = CatchEvent(cmd_menu,
        [&wrk, &state, caps, menu_index, form, selected_cmd](Event e) {
            if (e != Event::Return) return false;
            const auto* cmd = selected_cmd();
            if (!cmd) return true;
            auto idx = static_cast<std::size_t>(*menu_index);
            auto& cap = (*caps)[idx];
            if (cap->running) {
                cap->stop = true;
            } else {
                S::start_ctx ctx{ wrk, state, cap, *form };
                cmd->start(ctx);
            }
            return true;
        });

    // ── Input widgets, Maybe-gated per form_kind ──────────────────────

    auto make_input = [](std::string* s, const char* placeholder) {
        return Input(s, placeholder);
    };

    auto sym_in    = make_input(&form->symbol,   "symbol");
    auto pair_in   = make_input(&form->pair,     "pair");
    auto ival_in   = make_input(&form->interval, "interval (1m, 5m, 1h, …)");
    auto lvl_in    = make_input(&form->levels,   "levels (5, 10, 20)");
    auto speed_in  = make_input(&form->speed,    "speed (100ms, 250ms)");

    auto maybe = [selected_form_kind](auto widget, auto needs_fn) {
        return widget | Maybe([selected_form_kind, needs_fn] {
            return needs_fn(selected_form_kind());
        });
    };

    auto sym_m   = maybe(sym_in,   S::needs_symbol);
    auto pair_m  = maybe(pair_in,  S::needs_pair);
    auto ival_m  = maybe(ival_in,  S::needs_interval);
    auto lvl_m   = maybe(lvl_in,   S::needs_levels);
    auto speed_m = maybe(speed_in, S::needs_speed);

    auto form_container = Container::Vertical({
        sym_m, pair_m, ival_m, lvl_m, speed_m,
    });

    auto root = Container::Horizontal({
        cmd_menu_with_enter,
        form_container,
    });

    cmd_menu_with_enter->TakeFocus();

    auto view = Renderer(root, [cmd_menu_with_enter, form_container,
                                caps, menu_index, titles,
                                selected_cmd, selected_form_kind,
                                sym_m, pair_m, ival_m, lvl_m, speed_m,
                                models, cached_rows, cached_stamps, cached_widths,
                                event_box] {
        const auto* cmd = selected_cmd();
        const auto idx = static_cast<std::size_t>(*menu_index);
        const auto& cap = (*caps)[idx];
        const auto& m   = (*models)[idx];
        const bool running = cap->running.load();
        const auto total = cap->total_events.load();
        const auto errs  = cap->errors.load();

        // Rebuild the cached rows when the ring has changed or the
        // pane width changed. `total` is a monotonic counter so it's a
        // cheap "dirty" stamp — ring pops don't change it, but since
        // ring pops happen only when new events arrive, total advances
        // in lockstep.
        int outer_w = event_box->x_max - event_box->x_min + 1;
        int outer_h = event_box->y_max - event_box->y_min + 1;
        if (outer_w <= 0) outer_w = 40;
        if (outer_h <= 0) outer_h = 1;
        const int content_w = std::max(1, outer_w - 1);
        m->viewport_h = outer_h;
        m->viewport_w = content_w;

        if ((*cached_stamps)[idx] != total || (*cached_widths)[idx] != content_w) {
            std::deque<std::string> ring_snap;
            {
                std::lock_guard lk(cap->mtx);
                ring_snap = cap->ring;
            }
            auto& rows = (*cached_rows)[idx];
            rows.clear();
            // Newest-first order — matches the old view's behaviour.
            for (auto it = ring_snap.rbegin(); it != ring_snap.rend(); ++it) {
                for (auto& line : wrap_lines(*it, content_w))
                    rows.push_back(text(std::move(line)));
                // Blank separator between events.
                rows.push_back(text(""));
            }
            (*cached_stamps)[idx] = total;
            (*cached_widths)[idx] = content_w;
        }

        // Error banner + empty-state placeholder.
        std::string err_copy;
        {
            std::lock_guard lk(cap->mtx);
            err_copy = cap->error;
        }

        auto& rows = (*cached_rows)[idx];
        std::vector<Element> fallback;
        const std::vector<Element>* render_rows = &rows;
        if (rows.empty()) {
            fallback.push_back(
                text(running ? "  waiting for events…" : "  (no events)")
                | dim);
            render_rows = &fallback;
        }

        auto event_list = virtual_scroll_render(*render_rows, *m)
                        | reflect(*event_box)
                        | flex;

        // ── controls row ───────────────────────────────────────────
        Elements ctrl_rows;
        if (cmd) {
            ctrl_rows.push_back(text(cmd->name) | bold);
            ctrl_rows.push_back(text(cmd->description) | dim);
            ctrl_rows.push_back(separator());

            auto labeled = [](const char* label, Element field) {
                return hbox({ text(label) | dim | size(WIDTH, EQUAL, 12),
                              field | border | flex });
            };
            const auto fk = cmd->form;
            if (S::needs_symbol(fk))   ctrl_rows.push_back(labeled("symbol:",   sym_m->Render()));
            if (S::needs_pair(fk))     ctrl_rows.push_back(labeled("pair:",     pair_m->Render()));
            if (S::needs_interval(fk)) ctrl_rows.push_back(labeled("interval:", ival_m->Render()));
            if (S::needs_levels(fk))   ctrl_rows.push_back(labeled("levels:",   lvl_m->Render()));
            if (S::needs_speed(fk))    ctrl_rows.push_back(labeled("speed:",    speed_m->Render()));
        } else {
            ctrl_rows.push_back(text("(no selection)") | dim);
        }

        auto status_line = hbox({
            text(running ? " RUNNING " : " STOPPED ") | bold
                | (running ? bgcolor(Color::Green) | color(Color::Black)
                           : bgcolor(Color::GrayDark) | color(Color::White)),
            text("  events: " + std::to_string(total)),
            text("  errors: " + std::to_string(errs))
                | (errs > 0 ? color(Color::Red) : dim),
            filler(),
        });

        Elements middle_rows;
        for (auto& r : ctrl_rows) middle_rows.push_back(std::move(r));
        middle_rows.push_back(separator());
        middle_rows.push_back(status_line);
        if (!err_copy.empty()) {
            middle_rows.push_back(text("  ERROR: " + err_copy) | color(Color::Red));
        }
        middle_rows.push_back(filler());

        auto menu_block =
            vbox({ text("Streams") | bold, separator(),
                   cmd_menu_with_enter->Render() | vscroll_indicator | yframe | flex })
            | size(WIDTH, EQUAL, 22);

        return hbox({
            menu_block,
            separator(),
            vbox(std::move(middle_rows)) | size(WIDTH, EQUAL, 40),
            separator(),
            event_list,
        });
    });

    // ── Scroll handler (PageUp/Down + mouse wheel) ──────────────────

    auto component = CatchEvent(view, [models, menu_index, event_box](Event e) {
        auto idx = static_cast<std::size_t>(*menu_index);
        if (idx >= models->size()) return false;
        auto& m = (*models)[idx];

        const int max_top   = std::max(0, m->total_rows - m->viewport_h);
        const int step_page = std::max(1, m->viewport_h / 2);
        constexpr int step_wheel = 3;

        if (e == Event::PageDown) {
            m->scroll_top = std::min(max_top, m->scroll_top + step_page);
            return true;
        }
        if (e == Event::PageUp) {
            m->scroll_top = std::max(0, m->scroll_top - step_page);
            return true;
        }
        if (e.is_mouse()) {
            const auto& mouse = e.mouse();
            if (!event_box->Contain(mouse.x, mouse.y)) return false;
            if (mouse.button == Mouse::WheelUp) {
                m->scroll_top = std::max(0, m->scroll_top - step_wheel);
                return true;
            }
            if (mouse.button == Mouse::WheelDown) {
                m->scroll_top = std::min(max_top, m->scroll_top + step_wheel);
                return true;
            }
        }
        return false;
    });

    auto hints = [cmd_menu_with_enter, form_container]() -> std::vector<Element> {
        if (form_container->Focused()) {
            return {
                key_chip("↑↓", "next field"),
                key_chip("type", "edit"),
                key_chip("→/←", "zones"),
                key_chip("Tab", "cycle tabs"),
                key_chip("q", "quit"),
            };
        }
        if (cmd_menu_with_enter->Focused()) {
            return {
                key_chip("↑↓", "select stream"),
                key_chip("Enter", "start/stop"),
                key_chip("→", "form"),
                key_chip("PgUp/PgDn", "scroll events"),
                key_chip("Tab", "cycle tabs"),
                key_chip("q", "quit"),
            };
        }
        return {
            key_chip("↑↓", "select stream"),
            key_chip("Enter", "start/stop"),
            key_chip("PgUp/PgDn", "scroll events"),
            key_chip("Tab", "cycle tabs"),
            key_chip("q", "quit"),
        };
    };
    return tab_view{ component, std::move(hints) };
}

} // namespace demo_ui
