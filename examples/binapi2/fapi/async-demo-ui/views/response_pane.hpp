// SPDX-License-Identifier: Apache-2.0
//
// Shared Request / Response display components used by both the REST
// and WS API tabs. Top half = request, bottom half = response, each
// with Raw / JSON / Tree sub-tabs, scrollable via PageUp/PageDown.
//
// ── Why we virtualize ─────────────────────────────────────────────
//
// `yframe` renders its child's full content into a virtual screen and
// then clips — so for a 5000-line JSON response every frame pays for
// 5000 lines of layout + render even though only ~30 are visible.
// We replace that with manual virtualization:
//
//   1. Wrap / render content once per change into a
//      `std::vector<Element>` of rows (cached on `scroll_model`).
//   2. On each frame, read the pane's box via `reflect`, slice
//      `rows[scroll_top .. scroll_top + viewport_h]`, render only the
//      visible subset.
//   3. Draw our own scrollbar in the rightmost column (the usual
//      `vscroll_indicator` follows the child's focus, which we no
//      longer drive).
//
// PageUp/PageDown in the `CatchEvent` below moves `scroll_top` by
// exactly half the viewport — independent of total content size.

#pragma once

#include "../util/json_tree.hpp"
#include "../util/request_capture.hpp"
#include "../util/wrap_text.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;

/// Returns the currently-selected capture (depends on which command
/// is highlighted in the menu). Called at render time.
using get_cap_fn = std::function<std::shared_ptr<request_capture>()>;

namespace detail {

/// Per-sub-tab virtualized-scroll state. `viewport_*` and `total_rows`
/// are written by the Renderer once per frame (from `reflect` + cached
/// row count); `scroll_top` is updated by the CatchEvent handler.
struct scroll_model
{
    int scroll_top  = 0;
    int viewport_h  = 0;
    int viewport_w  = 0;   // content area width, excluding scrollbar column
    int total_rows  = 0;
};

/// Render a single row as a scrollbar cell. Thumb position/size reflect
/// where in the virtual list we're sitting. `vh` is the viewport height
/// in rows; `total` is the total row count.
inline Element scrollbar_column(int scroll_top, int viewport_h, int total)
{
    Elements cells;
    cells.reserve(static_cast<std::size_t>(viewport_h));
    if (total > viewport_h && viewport_h > 0) {
        const float ratio = static_cast<float>(viewport_h) / static_cast<float>(total);
        int thumb_h = std::max(1, static_cast<int>(ratio * static_cast<float>(viewport_h)));
        if (thumb_h > viewport_h) thumb_h = viewport_h;
        const int max_start = viewport_h - thumb_h;
        const int max_top   = std::max(1, total - viewport_h);
        int thumb_start = static_cast<int>(
            static_cast<float>(scroll_top) / static_cast<float>(max_top)
            * static_cast<float>(max_start) + 0.5f);
        thumb_start = std::clamp(thumb_start, 0, max_start);
        for (int i = 0; i < viewport_h; ++i) {
            const bool in_thumb = (i >= thumb_start && i < thumb_start + thumb_h);
            cells.push_back(text(in_thumb ? "█" : "│")
                            | color(in_thumb ? Color::GrayLight : Color::GrayDark));
        }
    } else {
        for (int i = 0; i < viewport_h; ++i) cells.push_back(text(" "));
    }
    return vbox(std::move(cells));
}

/// Render the virtualized content+scrollbar composite for this frame.
/// `rows` is the full (pre-rendered) list; we slice it to the visible
/// window and pad with empty rows if the content is shorter than the
/// viewport.
inline Element virtual_scroll_render(const std::vector<Element>& rows,
                                     scroll_model& m)
{
    const int total = static_cast<int>(rows.size());
    const int vh    = std::max(1, m.viewport_h);

    const int max_top = std::max(0, total - vh);
    m.scroll_top = std::clamp(m.scroll_top, 0, max_top);
    m.total_rows = total;

    const int end = std::min(m.scroll_top + vh, total);
    Elements visible;
    visible.reserve(static_cast<std::size_t>(vh));
    for (int i = m.scroll_top; i < end; ++i) visible.push_back(rows[static_cast<std::size_t>(i)]);
    while (static_cast<int>(visible.size()) < vh) visible.push_back(text(""));

    return hbox({
               vbox(std::move(visible)) | flex,
               scrollbar_column(m.scroll_top, vh, total),
           });
}

inline Component make_scrollable_text(std::function<std::string()> content_fn,
                                      std::shared_ptr<scroll_model> m,
                                      std::shared_ptr<Box> probe_box)
{
    auto cached_str   = std::make_shared<std::string>();
    auto cached_width = std::make_shared<int>(-1);
    auto cached_rows  = std::make_shared<std::vector<Element>>();

    return Renderer([content_fn, m, probe_box, cached_str, cached_width, cached_rows] {
        // `probe_box` was filled by `reflect` on the previous render of
        // any of the three sub-tabs (they share the same outer box).
        // Fallback is intentionally tiny so the first frame can't
        // overflow into the neighbouring layout — we lose a bit of
        // visible area for one frame, much better than spilling into
        // the bottom keybar.
        int outer_w = probe_box->x_max - probe_box->x_min + 1;
        int outer_h = probe_box->y_max - probe_box->y_min + 1;
        if (outer_w <= 0) outer_w = 40;
        if (outer_h <= 0) outer_h = 1;
        const int content_w = std::max(1, outer_w - 1); // -1 for scrollbar col

        m->viewport_h = outer_h;
        m->viewport_w = content_w;

        auto fresh = content_fn();
        if (fresh != *cached_str || *cached_width != content_w) {
            *cached_str   = std::move(fresh);
            *cached_width = content_w;
            auto lines = wrap_lines(*cached_str, content_w);
            if (lines.empty()) lines.push_back("");
            cached_rows->clear();
            cached_rows->reserve(lines.size());
            for (auto& l : lines) cached_rows->push_back(text(std::move(l)));
        }

        return virtual_scroll_render(*cached_rows, *m) | reflect(*probe_box);
    });
}

inline Component make_scrollable_tree(std::function<std::shared_ptr<glz::generic>()> tree_fn,
                                      std::shared_ptr<scroll_model> m,
                                      std::shared_ptr<Box> probe_box)
{
    auto cached_ptr  = std::make_shared<std::shared_ptr<glz::generic>>();
    auto cached_rows = std::make_shared<std::vector<Element>>();

    return Renderer([tree_fn, m, probe_box, cached_ptr, cached_rows] {
        int outer_w = probe_box->x_max - probe_box->x_min + 1;
        int outer_h = probe_box->y_max - probe_box->y_min + 1;
        if (outer_w <= 0) outer_w = 40;
        if (outer_h <= 0) outer_h = 1;
        m->viewport_h = outer_h;
        m->viewport_w = std::max(1, outer_w - 1);

        auto snapshot = tree_fn();
        if (snapshot != *cached_ptr) {
            *cached_ptr  = snapshot;
            *cached_rows = render_json_tree_rows(snapshot);
        }

        return virtual_scroll_render(*cached_rows, *m) | reflect(*probe_box);
    });
}

inline Component make_side_pane(get_cap_fn get_cap,
                                bool is_request,
                                const char* title)
{
    auto sub_tab_titles = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{ "Raw", "JSON", "Tree" });
    auto sub_tab_index = std::make_shared<int>(0);

    auto models = std::make_shared<std::array<std::shared_ptr<scroll_model>, 3>>(
        std::array<std::shared_ptr<scroll_model>, 3>{
            std::make_shared<scroll_model>(),
            std::make_shared<scroll_model>(),
            std::make_shared<scroll_model>(),
        });

    // One probe_box shared across the three sub-tabs. Container::Tab
    // only renders the active sub-tab, so without sharing, the
    // currently-inactive tabs would enter their first-render with a
    // stale zero box and momentarily overflow. With sharing, once any
    // sub-tab has rendered once, all three know the size.
    auto probe_box = std::make_shared<Box>();

    auto raw_view = make_scrollable_text([get_cap, is_request]() -> std::string {
        auto cap = get_cap();
        std::string error_msg;
        if (!is_request) {
            std::lock_guard lk(cap->mtx);
            if (!cap->error_message.empty()) {
                error_msg = "ERROR: " + cap->error_message;
                if (cap->http_status)
                    error_msg += "\n  http_status: " + std::to_string(cap->http_status);
                if (cap->binance_code)
                    error_msg += "\n  binance_code: " + std::to_string(cap->binance_code);
            }
        }
        capture_side side;
        {
            std::lock_guard lk(cap->mtx);
            side = is_request ? cap->request : cap->response;
        }
        std::string out = side.raw;
        if (!error_msg.empty()) {
            if (!out.empty()) out += "\n\n";
            out += error_msg;
        }
        if (out.empty())
            out = is_request ? "(no request yet)" : "(no response yet)";
        return out;
    }, (*models)[0], probe_box);

    auto json_view = make_scrollable_text([get_cap, is_request]() -> std::string {
        auto cap = get_cap();
        std::lock_guard lk(cap->mtx);
        const auto& side = is_request ? cap->request : cap->response;
        return side.pretty_json.empty()
            ? std::string{ "(no JSON yet)" }
            : side.pretty_json;
    }, (*models)[1], probe_box);

    auto tree_view = make_scrollable_tree([get_cap, is_request]() -> std::shared_ptr<glz::generic> {
        auto cap = get_cap();
        std::lock_guard lk(cap->mtx);
        return is_request ? cap->request.parsed_json : cap->response.parsed_json;
    }, (*models)[2], probe_box);

    auto tab_toggle = Toggle(sub_tab_titles.get(), sub_tab_index.get());
    auto tab_content = Container::Tab(
        { raw_view, json_view, tree_view },
        sub_tab_index.get());

    auto focusable = Container::Vertical({ tab_toggle });

    auto pane = Renderer(focusable, [tab_toggle, tab_content,
                                     sub_tab_titles, sub_tab_index, title] {
        return vbox({
                   hbox({ text(title) | bold | color(Color::Cyan),
                          text("  "),
                          tab_toggle->Render() }),
                   separator(),
                   tab_content->Render() | flex,
               });
    });

    return CatchEvent(pane, [models, sub_tab_index, probe_box](Event e) {
        auto idx = static_cast<std::size_t>(*sub_tab_index);
        if (idx >= models->size()) return false;
        auto& m = (*models)[idx];

        const int max_top    = std::max(0, m->total_rows - m->viewport_h);
        const int step_page  = std::max(1, m->viewport_h / 2);
        constexpr int step_wheel = 3;

        if (e == Event::PageDown) {
            m->scroll_top = std::min(max_top, m->scroll_top + step_page);
            return true;
        }
        if (e == Event::PageUp) {
            m->scroll_top = std::max(0, m->scroll_top - step_page);
            return true;
        }

        // Mouse wheel — only consume when the cursor is within this
        // side pane's box; otherwise let the event propagate (so the
        // menu / other pane can receive it if the mouse is over it).
        if (e.is_mouse()) {
            const auto& mouse = e.mouse();
            if (!probe_box->Contain(mouse.x, mouse.y)) return false;
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
}

} // namespace detail

/// Stacked Request / Response display: top = request, bottom = response.
/// 50/50 height split.
inline Component make_request_response_pane(get_cap_fn get_cap)
{
    auto request_pane  = detail::make_side_pane(get_cap, true,  "REQUEST");
    auto response_pane = detail::make_side_pane(get_cap, false, "RESPONSE");

    auto inner = Container::Vertical({ request_pane, response_pane });

    return Renderer(inner, [request_pane, response_pane] {
        auto terminal = ftxui::Terminal::Size();
        const int avail = std::max(6, terminal.dimy - 8);
        const int half = avail / 2;

        return vbox({
                   request_pane->Render()  | size(HEIGHT, EQUAL, half),
                   separator(),
                   response_pane->Render() | size(HEIGHT, EQUAL, half),
               });
    });
}

/// Pre-serialize a typed request struct into the capture's request side.
template<typename Request>
void prefill_request_json(request_capture& cap, const Request& req)
{
    auto j = glz::write<glz::opts{ .prettify = true }>(req);
    if (!j) return;
    std::lock_guard lk(cap.mtx);
    fill_pretty_and_parsed(cap.request, *j);
}

} // namespace demo_ui
