// SPDX-License-Identifier: Apache-2.0
//
// Renders a parsed `glz::generic` JSON value as a `tree(1)`-style
// display with box-drawing connectors:
//
//   {root}
//   ├── timezone: "UTC"
//   ├── serverTime: 1776280193368
//   ├── symbols [288 items]
//   │   ├── [0]
//   │   │   ├── symbol: "BTCUSDT"
//   │   │   ├── pair: "BTCUSDT"
//   │   │   └── status: "TRADING"
//   │   └── [1] {29 keys}
//   └── rateLimits [3 items]
//
// Depth-capped at `max_depth` (default 4). Row-capped at `max_rows`
// (default 500) to keep FTXUI's render path responsive even for large
// payloads like exchange-info.

#pragma once

#include <ftxui/dom/elements.hpp>

#include <glaze/glaze.hpp>
#include <glaze/json/generic.hpp>

#include <cstdio>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace demo_ui {

namespace detail {

inline constexpr int default_max_depth = 4;
inline constexpr std::size_t default_max_rows = 500;

/// Format a primitive value as a coloured text element.
inline ftxui::Element value_element(const glz::generic& value)
{
    using namespace ftxui;
    return std::visit([](auto const& v) -> Element {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>)
            return text("null") | color(Color::GrayDark);
        else if constexpr (std::is_same_v<T, bool>)
            return text(v ? "true" : "false") | color(Color::Magenta);
        else if constexpr (std::is_same_v<T, double>) {
            char buf[32];
            auto n = std::snprintf(buf, sizeof(buf), "%g", v);
            return text(std::string(buf, n > 0 ? n : 0)) | color(Color::Cyan);
        }
        else if constexpr (std::is_same_v<T, std::string>)
            return text("\"" + v + "\"") | color(Color::Green);
        else if constexpr (std::is_same_v<T, glz::generic::array_t>)
            return text("[" + std::to_string(v.size()) + " items]") | color(Color::GrayDark);
        else if constexpr (std::is_same_v<T, glz::generic::object_t>)
            return text("{" + std::to_string(v.size()) + " keys}") | color(Color::GrayDark);
        else
            return text("?");
    }, value.data);
}

/// Is this value a container (object or array) that would have children
/// at the current depth?
inline bool is_expandable(const glz::generic& value, int depth, int max_depth)
{
    if (depth >= max_depth) return false;
    return value.holds<glz::generic::object_t>()
        || value.holds<glz::generic::array_t>();
}

/// Recursively walk `value` and append tree-formatted rows to `out`.
///
/// `prefix` is the column of `│` / `   ` segments inherited from the
/// parent. Each child appends either `├── ` (non-last) or `└── `
/// (last) plus the child's connector column for its own children.
inline void render_node(const glz::generic& value,
                        const std::string& label,
                        const std::string& prefix,
                        bool is_last,
                        int depth,
                        int max_depth,
                        std::size_t max_rows,
                        std::vector<ftxui::Element>& out)
{
    using namespace ftxui;
    if (out.size() >= max_rows) return;

    // Connector for THIS row.
    const std::string connector = (depth == 0)
        ? ""
        : (is_last ? "└── " : "├── ");

    // Prefix for children: inherit parent's prefix + a vertical bar
    // (if we're not the last sibling) or spaces (if we are).
    const std::string child_prefix = (depth == 0)
        ? ""
        : prefix + (is_last ? "    " : "│   ");

    if (!is_expandable(value, depth, max_depth)) {
        // Leaf node (primitive, or container past depth cap).
        if (label.empty()) {
            out.push_back(hbox({
                text(prefix + connector) | color(Color::GrayDark),
                value_element(value),
            }));
        } else {
            out.push_back(hbox({
                text(prefix + connector) | color(Color::GrayDark),
                text(label) | bold,
                text(": "),
                value_element(value),
            }));
        }
        return;
    }

    // Container node — emit header then children.
    // Header: `├── key` or `├── [N]` or root `{root}`.
    if (depth == 0) {
        // Root node: just a label, no connector.
        if (value.holds<glz::generic::object_t>()) {
            out.push_back(text("{root}") | bold | color(Color::Cyan));
        } else {
            out.push_back(text("[root]") | bold | color(Color::Cyan));
        }
    } else if (label.empty()) {
        out.push_back(hbox({
            text(prefix + connector) | color(Color::GrayDark),
            value_element(value),
        }));
    } else {
        out.push_back(hbox({
            text(prefix + connector) | color(Color::GrayDark),
            text(label) | bold,
        }));
    }

    // Visit children.
    if (auto* obj = value.get_if<glz::generic::object_t>()) {
        std::size_t idx = 0;
        const std::size_t n = obj->size();
        for (auto const& [k, child] : *obj) {
            if (out.size() >= max_rows) break;
            render_node(child, k, child_prefix, idx + 1 == n,
                        depth + 1, max_depth, max_rows, out);
            ++idx;
        }
    } else if (auto* arr = value.get_if<glz::generic::array_t>()) {
        const std::size_t n = arr->size();
        for (std::size_t i = 0; i < n; ++i) {
            if (out.size() >= max_rows) break;
            render_node((*arr)[i], "[" + std::to_string(i) + "]",
                        child_prefix, i + 1 == n,
                        depth + 1, max_depth, max_rows, out);
        }
    }
}

} // namespace detail

/// Build the list of `tree(1)`-formatted rows for a parsed JSON value.
/// Returns a single-element vector with a placeholder if `value` is null.
/// Used by the virtualized scroll view which slices to the visible window.
inline std::vector<ftxui::Element>
render_json_tree_rows(const std::shared_ptr<glz::generic>& value,
                      int max_depth = detail::default_max_depth,
                      std::size_t max_rows = detail::default_max_rows)
{
    using namespace ftxui;
    std::vector<Element> rows;
    if (!value) {
        rows.push_back(text("(no parsed JSON)") | dim);
        return rows;
    }
    detail::render_node(*value, "", "", /*is_last=*/true,
                        0, max_depth, max_rows, rows);
    if (rows.size() >= max_rows) {
        rows.push_back(
            text("… (tree truncated at " + std::to_string(max_rows) + " rows)")
            | dim | color(Color::Yellow));
    }
    if (rows.empty()) rows.push_back(text("(empty)") | dim);
    return rows;
}

/// Convenience wrapper that bundles the rows into a vbox. Kept for any
/// caller that wants a single Element rather than the slicable list.
inline ftxui::Element render_json_tree(const std::shared_ptr<glz::generic>& value,
                                       int max_depth = detail::default_max_depth,
                                       std::size_t max_rows = detail::default_max_rows)
{
    return ftxui::vbox(render_json_tree_rows(value, max_depth, max_rows));
}

} // namespace demo_ui
