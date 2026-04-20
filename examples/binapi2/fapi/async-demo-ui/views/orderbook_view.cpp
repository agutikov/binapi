// SPDX-License-Identifier: Apache-2.0
//
// Order Book tab — step 2.
//
// Symbol + depth inputs, Start / Stop buttons, and a centred ASK/BID
// display showing the top 10 levels on each side with the spread in
// between.
//
// The `local_order_book` from `binapi2_fapi` does the heavy lifting:
// it subscribes to the diff-depth stream, fetches a REST snapshot,
// reconciles them, and calls our snapshot callback on every update.
// The callback writes into `book_capture` under a mutex and wakes the
// screen via `worker::notify_ui()`.

#include "../app_state.hpp"
#include "../book/book_capture.hpp"
#include "../worker.hpp"
#include "views.hpp"

#include <binapi2/fapi/order_book/local_order_book.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;
namespace types = binapi2::fapi::types;

namespace {

constexpr int display_levels = 10;

/// Notify-UI throttle: don't post more than ~30 Hz to avoid drowning
/// FTXUI's event queue under high-frequency book updates.
struct throttle
{
    std::chrono::steady_clock::time_point last{};
    bool should_notify()
    {
        auto now = std::chrono::steady_clock::now();
        if (now - last < std::chrono::milliseconds(33)) return false;
        last = now;
        return true;
    }
};

/// Free-function coroutine (per feedback_cobalt_lambda_lifetime.md).
/// Runs the local_order_book and fills `cap` via snapshot callback.
boost::cobalt::task<void>
book_coro(worker& wrk, std::shared_ptr<book_capture> cap)
{
    spdlog::info("orderbook: starting {} depth={}", cap->symbol, cap->depth);

    spdlog::debug("orderbook: creating market_stream");
    auto streams = wrk.api().create_market_stream();

    spdlog::debug("orderbook: acquiring REST client");
    auto* rest = co_await wrk.acquire_rest_client_raw();
    if (!rest) {
        spdlog::error("orderbook: REST connect failed");
        {
            std::lock_guard lk(cap->mtx);
            cap->error = "REST connect failed";
        }
        cap->running = false;
        wrk.notify_ui();
        co_return;
    }
    spdlog::debug("orderbook: REST client acquired");

    spdlog::debug("orderbook: constructing local_order_book");
    binapi2::fapi::order_book::local_order_book book(*streams, rest->market_data);

    {
        std::lock_guard lk(cap->mtx);
        cap->book_ptr = &book;
    }

    auto thr = std::make_shared<throttle>();

    book.set_snapshot_callback([cap, &wrk, thr](const binapi2::fapi::order_book::order_book_snapshot& snap) {
        {
            std::lock_guard lk(cap->mtx);
            cap->latest = snap;
        }
        ++cap->updates;
        if (thr->should_notify()) wrk.notify_ui();
    });

    spdlog::info("orderbook: calling async_run({}, {})", cap->symbol, cap->depth);
    auto r = co_await book.async_run(types::symbol_t{ cap->symbol }, cap->depth);

    spdlog::info("orderbook: async_run returned, ok={}", static_cast<bool>(r));
    {
        std::lock_guard lk(cap->mtx);
        cap->book_ptr = nullptr;
        if (!r) {
            cap->error = r.err.message;
            spdlog::error("orderbook: error: {}", r.err.message);
        }
    }
    cap->running = false;
    wrk.notify_ui();
    spdlog::info("orderbook: stopped (updates={})", cap->updates.load());
}

} // namespace

tab_view make_orderbook_view(app_state& /*state*/, worker& wrk)
{
    auto cap = std::make_shared<book_capture>();

    auto symbol = std::make_shared<std::string>("BTCUSDT");
    auto depth_str = std::make_shared<std::string>("1000");

    auto symbol_input = Input(symbol.get(), "symbol");
    auto depth_input  = Input(depth_str.get(), "depth");

    auto start_button = Button("Start", [&wrk, cap, symbol, depth_str] {
        if (cap->running) return;
        cap->symbol = *symbol;
        cap->depth = std::stoi(*depth_str);
        cap->updates = 0;
        {
            std::lock_guard lk(cap->mtx);
            cap->latest = {};
            cap->error.clear();
        }
        cap->running = true;
        boost::cobalt::spawn(
            wrk.io().get_executor(),
            book_coro(wrk, cap),
            boost::asio::use_future);
    });

    auto stop_button = Button("Stop", [cap] {
        std::lock_guard lk(cap->mtx);
        if (cap->book_ptr) cap->book_ptr->stop();
    });

    auto controls = Container::Horizontal({
        symbol_input,
        depth_input,
        start_button,
        stop_button,
    });

    auto component = Renderer(controls, [controls, cap, symbol_input, depth_input,
                               start_button, stop_button, symbol, depth_str] {
        // Copy snapshot under lock.
        binapi2::fapi::order_book::order_book_snapshot snap;
        std::string err;
        {
            std::lock_guard lk(cap->mtx);
            snap = cap->latest;
            err = cap->error;
        }
        const bool running = cap->running.load();
        const auto updates = cap->updates.load();

        // ── controls row ──────────────────────────────────────────
        auto ctrl_row = hbox({
            text(" symbol: ") | dim,
            symbol_input->Render() | size(WIDTH, EQUAL, 12) | border,
            text("  depth: ") | dim,
            depth_input->Render() | size(WIDTH, EQUAL, 8) | border,
            text("  "),
            start_button->Render(),
            text(" "),
            stop_button->Render(),
            text("  "),
            text(running ? "running" : "stopped") | bold
                | color(running ? Color::Green : Color::GrayDark),
            text("  updates: " + std::to_string(updates)) | dim,
        });

        if (!err.empty()) {
            ctrl_row = vbox({ ctrl_row,
                              text("  ERROR: " + err) | color(Color::Red) });
        }

        // ── book display ──────────────────────────────────────────
        if (snap.bids.empty() && snap.asks.empty()) {
            return vbox({
                ctrl_row,
                separator(),
                text(running ? "  waiting for snapshot…" : "  (no data)") | dim | flex,
            });
        }

        // Collect top N asks (ascending by price → display reversed)
        // and top N bids (descending by price).
        std::vector<std::pair<types::decimal_t, types::decimal_t>> top_asks;
        {
            auto it = snap.asks.begin();
            for (int i = 0; i < display_levels && it != snap.asks.end(); ++i, ++it)
                top_asks.emplace_back(it->first, it->second);
        }

        std::vector<std::pair<types::decimal_t, types::decimal_t>> top_bids;
        {
            auto it = snap.bids.begin();
            for (int i = 0; i < display_levels && it != snap.bids.end(); ++i, ++it)
                top_bids.emplace_back(it->first, it->second);
        }

        // Find max qty for bar scaling.
        types::decimal_t max_qty;
        for (auto& [p, q] : top_asks) if (q > max_qty) max_qty = q;
        for (auto& [p, q] : top_bids) if (q > max_qty) max_qty = q;

        auto bar_width = [&](const types::decimal_t& qty) -> int {
            if (max_qty.is_zero()) return 0;
            // Simplified: parse as double for the bar ratio.
            double q = std::stod(qty.to_string());
            double m = std::stod(max_qty.to_string());
            return static_cast<int>(q / m * 20.0);
        };

        auto make_row = [&](const types::decimal_t& price,
                            const types::decimal_t& qty,
                            Color row_color) -> Element {
            int bw = bar_width(qty);
            return hbox({
                text(std::string(20 - std::min(bw, 20), ' ') + std::string(std::min(bw, 20), '#'))
                    | color(row_color) | dim,
                text("  "),
                text(price.to_string()) | bold | size(WIDTH, EQUAL, 16),
                text("  "),
                text(qty.to_string()) | size(WIDTH, EQUAL, 16),
            });
        };

        Elements rows;
        rows.push_back(hbox({
            text("                      ") | dim,
            text("  "),
            text("Price") | bold | underlined | size(WIDTH, EQUAL, 16),
            text("  "),
            text("Quantity") | bold | underlined | size(WIDTH, EQUAL, 16),
        }));
        rows.push_back(separator());

        // Asks: reversed so best ask is at the bottom, near the spread.
        for (auto it = top_asks.rbegin(); it != top_asks.rend(); ++it)
            rows.push_back(make_row(it->first, it->second, Color::Red));

        // Spread line.
        if (!top_asks.empty() && !top_bids.empty()) {
            auto best_ask = top_asks.front().first;
            auto best_bid = top_bids.front().first;
            // Compute spread as string subtraction via double (good enough for display).
            double spread = std::stod(best_ask.to_string()) - std::stod(best_bid.to_string());
            double mid = (std::stod(best_ask.to_string()) + std::stod(best_bid.to_string())) / 2.0;
            char buf[64];
            std::snprintf(buf, sizeof(buf), "spread %.2f  mid %.2f", spread, mid);
            rows.push_back(separator());
            rows.push_back(text(std::string("  ── ") + buf + " ──") | bold | color(Color::Yellow));
            rows.push_back(separator());
        }

        // Bids: best bid at top, near the spread.
        for (auto& [p, q] : top_bids)
            rows.push_back(make_row(p, q, Color::Green));

        rows.push_back(separator());
        rows.push_back(text("  lastUpdateId: " + std::to_string(snap.last_update_id)) | dim);

        return vbox({
            ctrl_row,
            separator(),
            vbox(std::move(rows)) | flex,
        });
    });

    auto hints = []() -> std::vector<Element> {
        return {
            key_chip("Tab", "next control"),
            key_chip("Enter", "press button"),
            key_chip("type", "edit symbol/depth"),
            key_chip("q", "quit"),
        };
    };
    return tab_view{ component, std::move(hints) };
}

} // namespace demo_ui
