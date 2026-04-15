// SPDX-License-Identifier: Apache-2.0
//
// Renders a parsed `glz::generic` JSON value as a tree of indented FTXUI
// elements. Step 1 keeps it static (just nested `text()` rows): once the
// REST plumbing is solid in step 4 we can upgrade to interactive
// `Collapsible` components.
//
// `render_json_tree(value)` returns an `Element` ready to drop into a
// `vbox`. It caps recursion at `max_depth` to keep large responses
// (`exchange-info`) from blowing up the dom tree — beyond the cap the
// node renders as `"… {N more}"`.

#pragma once

#include <ftxui/dom/elements.hpp>

#include <glaze/glaze.hpp>
#include <glaze/json/generic.hpp>

#include <memory>
#include <string>
#include <variant>

namespace demo_ui {

namespace detail {

inline constexpr int default_max_depth = 4;

/// Walk `value` and append rendered rows to `out`. Indentation is two
/// spaces per level. Object/array values get a header row (`key {` /
/// `key [`) and child rows; primitives get a single `key: value` row.
inline void render_node(const glz::generic& value,
                        std::string label,
                        int depth,
                        int max_depth,
                        std::vector<ftxui::Element>& out)
{
    using namespace ftxui;

    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');
    auto leaf = [&](std::string text_value, Color value_color) {
        out.push_back(hbox({
            text(indent + label),
            text(label.empty() ? "" : ": "),
            text(std::move(text_value)) | color(value_color),
        }));
    };

    std::visit([&](auto const& v) {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            leaf("null", Color::GrayDark);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            leaf(v ? "true" : "false", Color::Magenta);
        }
        else if constexpr (std::is_same_v<T, double>) {
            // Strip trailing zeros for integer-valued doubles so JSON
            // numbers like 0 / 1 don't render as "0.000000".
            char buf[32];
            const auto written = std::snprintf(buf, sizeof(buf), "%g", v);
            leaf(std::string(buf, buf + (written > 0 ? written : 0)), Color::Cyan);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            leaf("\"" + v + "\"", Color::Green);
        }
        else if constexpr (std::is_same_v<T, glz::generic::array_t>) {
            if (depth >= max_depth) {
                leaf("[" + std::to_string(v.size()) + " items]", Color::GrayDark);
                return;
            }
            out.push_back(text(indent + label + (label.empty() ? "[" : ": [")));
            for (std::size_t i = 0; i < v.size(); ++i) {
                render_node(v[i], "[" + std::to_string(i) + "]",
                            depth + 1, max_depth, out);
            }
            out.push_back(text(indent + "]"));
        }
        else if constexpr (std::is_same_v<T, glz::generic::object_t>) {
            if (depth >= max_depth) {
                leaf("{" + std::to_string(v.size()) + " keys}", Color::GrayDark);
                return;
            }
            out.push_back(text(indent + label + (label.empty() ? "{" : ": {")));
            for (auto const& [k, child] : v) {
                render_node(child, k, depth + 1, max_depth, out);
            }
            out.push_back(text(indent + "}"));
        }
    }, value.data);
}

} // namespace detail

/// Build a vbox of indented rows for a parsed JSON value.
/// Returns a `text("(no parsed JSON)")` placeholder if `value` is null.
inline ftxui::Element render_json_tree(const std::shared_ptr<glz::generic>& value,
                                       int max_depth = detail::default_max_depth)
{
    using namespace ftxui;
    if (!value) return text("(no parsed JSON)") | dim;

    std::vector<Element> rows;
    detail::render_node(*value, "", 0, max_depth, rows);
    if (rows.empty()) return text("(empty)") | dim;
    return vbox(std::move(rows));
}

} // namespace demo_ui
