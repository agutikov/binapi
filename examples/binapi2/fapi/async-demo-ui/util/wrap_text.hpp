// SPDX-License-Identifier: Apache-2.0
//
// Word-wrap helper that produces a plain `vbox` of `text` elements.
//
// We used to wrap via FTXUI's `paragraph`, which for every source line
// builds a flexbox with one text element per word. That's fine for short
// content, but a JSON response with thousands of lines ends up with
// thousands of flexboxes plus alignment/justification layout per frame,
// which the user noticed as sluggish redraws. Pre-wrapping to a
// `vector<string>` and emitting plain `text(line)` rows is the same
// visual result with ~10× less per-frame work.
//
// Rules:
//   * `\n` in the input forces a line break (preserved as-is).
//   * Each non-empty line is greedily packed up to `width` characters,
//     breaking at the last whitespace within the window.
//   * If a single token is wider than `width`, it's force-broken.
//   * Whitespace at a wrap point is dropped (standard word-wrap
//     behaviour); explicit whitespace at the start of a line is kept
//     for indent preservation on the first wrapped row.

#pragma once

#include <ftxui/dom/elements.hpp>

#include <cctype>
#include <string>
#include <vector>

namespace demo_ui {

namespace detail {

/// Wrap one logical (newline-free) line into zero-or-more output lines.
inline void wrap_single_line(const std::string& line, int width,
                             std::vector<std::string>& out)
{
    if (width <= 0 || static_cast<int>(line.size()) <= width) {
        out.push_back(line);
        return;
    }
    std::size_t i = 0;
    while (i < line.size()) {
        std::size_t end = std::min(i + static_cast<std::size_t>(width), line.size());
        if (end >= line.size()) {
            out.push_back(line.substr(i));
            return;
        }
        // Try to break at the last whitespace in (i, end].
        std::size_t back = end;
        while (back > i &&
               !std::isspace(static_cast<unsigned char>(line[back - 1])))
            --back;
        if (back > i) {
            // Wrap at the whitespace position.
            out.push_back(line.substr(i, back - 1 - i));
            i = back; // consume the whitespace char itself
        } else {
            // No whitespace in this window — force-break at `width`.
            out.push_back(line.substr(i, end - i));
            i = end;
        }
    }
}

} // namespace detail

/// Wrap `content` into a flat list of strings, one per output line.
/// Preserves explicit `\n` boundaries; word-boundary wraps each long
/// logical line and force-breaks any token wider than `width`.
inline std::vector<std::string> wrap_lines(const std::string& content, int width)
{
    std::vector<std::string> out;
    std::size_t pos = 0;
    while (true) {
        std::size_t nl = content.find('\n', pos);
        std::size_t end = (nl == std::string::npos) ? content.size() : nl;
        detail::wrap_single_line(content.substr(pos, end - pos), width, out);
        if (nl == std::string::npos) break;
        pos = nl + 1;
    }
    return out;
}

/// Word-wrapped paragraph, rendered as a plain `vbox` of `text` rows.
/// `width` caps each row's length; tokens longer than `width` are
/// force-broken.
inline ftxui::Element wrap_paragraph(const std::string& content,
                                     int width = 40)
{
    using namespace ftxui;
    auto lines = wrap_lines(content, width);
    if (lines.empty()) lines.push_back("");
    std::vector<Element> rows;
    rows.reserve(lines.size());
    for (auto& line : lines) rows.push_back(text(std::move(line)));
    return vbox(std::move(rows));
}

} // namespace demo_ui
