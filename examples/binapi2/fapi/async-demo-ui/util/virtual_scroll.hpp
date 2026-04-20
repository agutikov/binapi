// SPDX-License-Identifier: Apache-2.0
//
// Virtualized scroll helpers — shared between the REST/WS response panes
// and the Streams event list.
//
// Why virtualize: FTXUI's `yframe` renders the full child content into a
// virtual screen and then clips to the visible box. For a 5000-line JSON
// response every frame paid for 5000 lines of layout + render even though
// only ~30 were visible. Here we instead:
//
//   1. Pre-render content once per change into a `std::vector<Element>`
//      (the caller's cache — one `text(line)` per wrapped line, or one
//      Element per tree row).
//   2. On each frame, read the pane's box via `reflect`, slice
//      `rows[scroll_top .. scroll_top + viewport_h]`, render only the
//      visible subset.
//   3. Draw a 1-column scrollbar on the right with a proportionally-sized
//      thumb (`█` + `│`).
//
// Consumers:
//   * `views/response_pane.hpp` — one scroll_model per sub-tab (Raw /
//     JSON / Tree), one probe_box shared across all three.
//   * `views/streams_view.cpp` — one scroll_model per stream; the rows
//     are the pre-wrapped lines of the event ring.

#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>

#include <algorithm>
#include <vector>

namespace demo_ui {

/// Per-view virtualized-scroll state. `viewport_*` and `total_rows`
/// are written by the Renderer once per frame (from `reflect` + the
/// cached row count); `scroll_top` is updated by the event handler.
struct scroll_model
{
    int scroll_top = 0;
    int viewport_h = 0;
    int viewport_w = 0;   // content area width, excluding scrollbar column
    int total_rows = 0;
};

/// One column of cells showing a scrollbar with a thumb proportional to
/// viewport/total. Produces exactly `viewport_h` rows.
inline ftxui::Element scrollbar_column(int scroll_top, int viewport_h, int total)
{
    using namespace ftxui;
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

/// Slice `rows` to the visible window and emit the content + scrollbar.
/// Updates `m.scroll_top` (clamping) and `m.total_rows` as a side effect.
inline ftxui::Element virtual_scroll_render(const std::vector<ftxui::Element>& rows,
                                            scroll_model& m)
{
    using namespace ftxui;
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

} // namespace demo_ui
