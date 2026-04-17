// SPDX-License-Identifier: Apache-2.0
//
// Shared Request / Response display components used by both the REST
// and WS API tabs. Top half = request, bottom half = response, each
// with Raw / JSON / Tree sub-tabs, scrollable via PageUp/PageDown.

#pragma once

#include "../util/json_tree.hpp"
#include "../util/request_capture.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

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

inline constexpr float page_step = 0.15f;

inline Component make_scrollable_text(std::function<std::string()> content_fn,
                                      std::shared_ptr<float> scroll)
{
    auto cached_str  = std::make_shared<std::string>();
    auto cached_elem = std::make_shared<Element>();

    return Renderer([content_fn, scroll, cached_str, cached_elem] {
        auto fresh = content_fn();
        if (fresh != *cached_str) {
            *cached_str = std::move(fresh);
            *cached_elem = paragraph(*cached_str);
        }
        return *cached_elem
            | focusPositionRelative(0.f, *scroll)
            | yframe
            | vscroll_indicator
            | flex;
    });
}

inline Component make_scrollable_tree(std::function<std::shared_ptr<glz::generic>()> tree_fn,
                                      std::shared_ptr<float> scroll)
{
    auto cached_ptr  = std::make_shared<std::shared_ptr<glz::generic>>();
    auto cached_elem = std::make_shared<Element>();

    return Renderer([tree_fn, scroll, cached_ptr, cached_elem] {
        auto snapshot = tree_fn();
        if (snapshot != *cached_ptr) {
            *cached_ptr = snapshot;
            *cached_elem = render_json_tree(snapshot);
        }
        return *cached_elem
            | focusPositionRelative(0.f, *scroll)
            | yframe
            | vscroll_indicator
            | flex;
    });
}

inline Component make_side_pane(get_cap_fn get_cap,
                                bool is_request,
                                const char* title)
{
    auto sub_tab_titles = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{ "Raw", "JSON", "Tree" });
    auto sub_tab_index = std::make_shared<int>(0);

    auto scrolls = std::make_shared<std::array<std::shared_ptr<float>, 3>>(
        std::array<std::shared_ptr<float>, 3>{
            std::make_shared<float>(0.f),
            std::make_shared<float>(0.f),
            std::make_shared<float>(0.f),
        });

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
    }, (*scrolls)[0]);

    auto json_view = make_scrollable_text([get_cap, is_request]() -> std::string {
        auto cap = get_cap();
        std::lock_guard lk(cap->mtx);
        const auto& side = is_request ? cap->request : cap->response;
        return side.pretty_json.empty()
            ? std::string{ "(no JSON yet)" }
            : side.pretty_json;
    }, (*scrolls)[1]);

    auto tree_view = make_scrollable_tree([get_cap, is_request]() -> std::shared_ptr<glz::generic> {
        auto cap = get_cap();
        std::lock_guard lk(cap->mtx);
        return is_request ? cap->request.parsed_json : cap->response.parsed_json;
    }, (*scrolls)[2]);

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

    return CatchEvent(pane, [scrolls, sub_tab_index](Event e) {
        auto idx = static_cast<std::size_t>(*sub_tab_index);
        if (idx >= scrolls->size()) return false;
        auto& scroll = (*scrolls)[idx];
        if (e == Event::PageDown) { *scroll = std::min(1.f, *scroll + page_step); return true; }
        if (e == Event::PageUp)   { *scroll = std::max(0.f, *scroll - page_step); return true; }
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
