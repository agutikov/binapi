// SPDX-License-Identifier: Apache-2.0
//
// Order Book tab.
//
// Split screen: trades tape on the LEFT, order book on the RIGHT. Tape
// rows are laid out with price as the rightmost column so the price
// lines up visually adjacent to the book's price column across the
// vertical separator:
//
//   ┌───── Trades Tape ─────┬──── Order Book ─────┐
//   │ qty    side   time   price │      ASK 29412.20 │
//   │ 0.412  B      23.4   29412.10 │   ASK 29412.10 │
//   │ ...                   │   ── spread / mid ──   │
//   │                       │      BID 29412.00      │
//   └───────────────────────┴────────────────────────┘
//
// Start spawns two tasks: the existing `local_order_book` coroutine and
// an `aggregate_trade` stream subscription. Stop flips both stop flags.

#include "../app_state.hpp"
#include "../book/book_capture.hpp"
#include "../book/trades_tape_capture.hpp"
#include "../worker.hpp"
#include "views.hpp"

#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/demo_commands/result_sink.hpp>
#include <binapi2/fapi/order_book/local_order_book.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <glaze/glaze.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;
namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

constexpr int display_levels = 10;

/// Notify-UI throttle: don't post more than ~30 Hz to avoid drowning
/// FTXUI's event queue under high-frequency updates.
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

// ── Book coroutine (unchanged from step-2 logic) ─────────────────────

boost::cobalt::task<void>
book_coro(worker& wrk, std::shared_ptr<book_capture> cap)
{
    spdlog::info("orderbook: starting {} depth={}", cap->symbol, cap->depth);

    auto streams = wrk.api().create_market_stream();

    auto* rest = co_await wrk.acquire_rest_client_raw();
    if (!rest) {
        {
            std::lock_guard lk(cap->mtx);
            cap->error = "REST connect failed";
        }
        cap->running = false;
        wrk.notify_ui();
        co_return;
    }

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

    auto r = co_await book.async_run(types::symbol_t{ cap->symbol }, cap->depth);
    {
        std::lock_guard lk(cap->mtx);
        cap->book_ptr = nullptr;
        if (!r) cap->error = r.err.message;
    }
    cap->running = false;
    wrk.notify_ui();
}

// ── Tape sink ────────────────────────────────────────────────────────

class tape_sink final : public lib::result_sink
{
public:
    tape_sink(std::shared_ptr<trades_tape_capture> cap,
              worker& wrk,
              app_state& state)
        : cap_(std::move(cap))
        , worker_(wrk)
        , state_(state)
    {
        ++state_.active_jobs;
        worker_.notify_ui();
    }

    ~tape_sink() override
    {
        --state_.active_jobs;
        worker_.notify_ui();
    }

    void on_info(std::string_view /*msg*/) override {}

    void on_response_json(std::string_view pretty) override
    {
        // Parse pretty JSON back into the typed event. exec_stream hands
        // us the pretty JSON only, so to get typed fields we re-parse.
        // Pretty JSON is what we have; a cheaper path would be to bypass
        // exec_stream and subscribe directly, but this keeps the layout
        // aligned with the other tabs.
        types::aggregate_trade_stream_event_t evt;
        if (glz::read_json(evt, pretty) != glz::error_code::none) {
            ++cap_->errors;
            return;
        }

        tape_trade t{ evt.price, evt.quantity, evt.trade_time, evt.is_buyer_maker };
        {
            std::lock_guard lk(cap_->mtx);
            cap_->ring.push_back(std::move(t));
            while (cap_->ring.size() > trades_tape_capture::max_ring)
                cap_->ring.pop_front();
        }
        ++cap_->total;
        throttled_notify();
    }

    void on_error(const binapi2::fapi::error& err) override
    {
        {
            std::lock_guard lk(cap_->mtx);
            cap_->error = err.message;
        }
        ++cap_->errors;
        worker_.notify_ui();
    }

    void on_done(int /*rc*/) override {}

private:
    void throttled_notify()
    {
        auto now = std::chrono::steady_clock::now();
        if (now - last_notify_ < std::chrono::milliseconds(33)) return;
        last_notify_ = now;
        worker_.notify_ui();
    }

    std::shared_ptr<trades_tape_capture> cap_;
    worker& worker_;
    app_state& state_;
    std::chrono::steady_clock::time_point last_notify_{};
};

// ── Tape coroutine ───────────────────────────────────────────────────

boost::cobalt::task<void>
tape_coro(worker& wrk,
          std::shared_ptr<tape_sink> sink,
          std::shared_ptr<trades_tape_capture> cap,
          std::string symbol)
{
    spdlog::info("tape: starting aggTrade {}", symbol);
    types::aggregate_trade_subscription sub;
    sub.symbol = symbol;
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
    spdlog::info("tape: stopped");
}

// ── Helpers ──────────────────────────────────────────────────────────

std::string format_time(types::timestamp_ms_t ts)
{
    const auto ms = ts.value % 1000;
    const auto sec = std::time_t(ts.value / 1000);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &sec);
#else
    localtime_r(&sec, &tm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03lld",
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<long long>(ms));
    return buf;
}

Element render_tape(const std::vector<tape_trade>& snap)
{
    Elements rows;
    rows.push_back(hbox({
        text("qty")    | bold | underlined | size(WIDTH, EQUAL, 10),
        text(" side ") | bold | underlined,
        text(" time ")     | bold | underlined | size(WIDTH, EQUAL, 14),
        text("price")  | bold | underlined | flex,
    }));
    rows.push_back(separator());

    // newest-first display: tape was appended newest-at-back, so reverse.
    for (auto it = snap.rbegin(); it != snap.rend(); ++it) {
        const auto& t = *it;
        // is_buyer_maker == true → the aggressive side was a seller.
        const bool seller_aggressor = t.is_buyer_maker;
        const Color row_color = seller_aggressor ? Color::Red : Color::Green;
        const char* side_ch   = seller_aggressor ? "S" : "B";
        rows.push_back(hbox({
            text(t.quantity.to_string()) | color(row_color) | size(WIDTH, EQUAL, 10),
            text(" "),
            text(side_ch) | bold | color(row_color),
            text("   "),
            text(format_time(t.time)) | dim | size(WIDTH, EQUAL, 14),
            text(t.price.to_string()) | color(row_color) | flex,
        }));
    }
    return vbox(std::move(rows)) | yframe | flex;
}

Element render_book(const binapi2::fapi::order_book::order_book_snapshot& snap)
{
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

    types::decimal_t max_qty;
    for (auto& [p, q] : top_asks) if (q > max_qty) max_qty = q;
    for (auto& [p, q] : top_bids) if (q > max_qty) max_qty = q;

    auto bar_width = [&](const types::decimal_t& qty) -> int {
        if (max_qty.is_zero()) return 0;
        double q = std::stod(qty.to_string());
        double m = std::stod(max_qty.to_string());
        return static_cast<int>(q / m * 16.0);
    };

    auto row = [&](const types::decimal_t& price,
                   const types::decimal_t& qty,
                   Color row_color,
                   bool top_of_book) -> Element {
        const int bw = bar_width(qty);
        auto price_text = text(price.to_string());
        if (top_of_book) price_text = std::move(price_text) | bold;
        return hbox({
            text(" "),
            price_text | color(row_color) | size(WIDTH, EQUAL, 12),
            text("  "),
            text(std::string(std::min(bw, 16), '#')) | color(row_color) | dim,
            text(std::string(16 - std::min(bw, 16), ' ')),
            text("  "),
            text(qty.to_string()) | size(WIDTH, EQUAL, 12),
        });
    };

    Elements rows;
    rows.push_back(hbox({
        text(" "),
        text("Price") | bold | underlined | size(WIDTH, EQUAL, 12),
        text("  "),
        text("  bar (qty)  ") | bold | underlined | size(WIDTH, EQUAL, 18),
        text("  "),
        text("Quantity") | bold | underlined | size(WIDTH, EQUAL, 12),
    }));
    rows.push_back(separator());

    // Asks reversed so best ask is at the bottom near the spread.
    for (auto it = top_asks.rbegin(); it != top_asks.rend(); ++it)
        rows.push_back(row(it->first, it->second, Color::Red,
                           /*top_of_book=*/it + 1 == top_asks.rend()));

    if (!top_asks.empty() && !top_bids.empty()) {
        const auto best_ask = top_asks.front().first;
        const auto best_bid = top_bids.front().first;
        const double spread = std::stod(best_ask.to_string()) - std::stod(best_bid.to_string());
        const double mid    = (std::stod(best_ask.to_string()) + std::stod(best_bid.to_string())) / 2.0;
        char buf[64];
        std::snprintf(buf, sizeof(buf), "spread %.2f  mid %.2f", spread, mid);
        rows.push_back(separator());
        rows.push_back(text(std::string("  ── ") + buf + " ──")
                       | bold | color(Color::Yellow));
        rows.push_back(separator());
    }

    for (std::size_t i = 0; i < top_bids.size(); ++i)
        rows.push_back(row(top_bids[i].first, top_bids[i].second,
                           Color::Green, /*top_of_book=*/i == 0));

    return vbox(std::move(rows)) | flex;
}

} // namespace

tab_view make_orderbook_view(app_state& state, worker& wrk)
{
    auto book_cap = std::make_shared<book_capture>();
    auto tape_cap = std::make_shared<trades_tape_capture>();

    auto symbol    = std::make_shared<std::string>("BTCUSDT");
    auto depth_str = std::make_shared<std::string>("1000");

    auto symbol_input = Input(symbol.get(), "symbol");
    auto depth_input  = Input(depth_str.get(), "depth");

    auto start_button = Button("Start", [&wrk, &state, book_cap, tape_cap,
                                         symbol, depth_str] {
        if (book_cap->running) return;
        book_cap->symbol = *symbol;
        try { book_cap->depth = std::stoi(*depth_str); } catch (...) { book_cap->depth = 1000; }
        book_cap->updates = 0;
        {
            std::lock_guard lk(book_cap->mtx);
            book_cap->latest = {};
            book_cap->error.clear();
        }
        book_cap->running = true;

        tape_cap->stop = false;
        tape_cap->total = 0;
        tape_cap->errors = 0;
        {
            std::lock_guard lk(tape_cap->mtx);
            tape_cap->ring.clear();
            tape_cap->error.clear();
        }

        boost::cobalt::spawn(wrk.io().get_executor(),
                             book_coro(wrk, book_cap),
                             boost::asio::use_future);
        auto sink = std::make_shared<tape_sink>(tape_cap, wrk, state);
        boost::cobalt::spawn(wrk.io().get_executor(),
                             tape_coro(wrk, std::move(sink), tape_cap, *symbol),
                             boost::asio::use_future);
    });

    auto stop_button = Button("Stop", [book_cap, tape_cap] {
        {
            std::lock_guard lk(book_cap->mtx);
            if (book_cap->book_ptr) book_cap->book_ptr->stop();
        }
        tape_cap->stop = true;
    });

    auto controls = Container::Horizontal({
        symbol_input,
        depth_input,
        start_button,
        stop_button,
    });

    auto component = Renderer(controls, [controls, book_cap, tape_cap,
                                         symbol_input, depth_input,
                                         start_button, stop_button] {
        binapi2::fapi::order_book::order_book_snapshot snap;
        std::string book_err;
        {
            std::lock_guard lk(book_cap->mtx);
            snap = book_cap->latest;
            book_err = book_cap->error;
        }
        std::vector<tape_trade> tape_snap;
        std::string tape_err;
        {
            std::lock_guard lk(tape_cap->mtx);
            tape_snap.assign(tape_cap->ring.begin(), tape_cap->ring.end());
            tape_err = tape_cap->error;
        }

        const bool running = book_cap->running.load();
        const auto book_updates = book_cap->updates.load();
        const auto trades_total = tape_cap->total.load();

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
            text("  book updates: " + std::to_string(book_updates)) | dim,
            text("  trades: "       + std::to_string(trades_total)) | dim,
        });

        Elements errors;
        if (!book_err.empty()) errors.push_back(text("  book error: "  + book_err)  | color(Color::Red));
        if (!tape_err.empty()) errors.push_back(text("  tape error: "  + tape_err)  | color(Color::Red));

        if (snap.bids.empty() && snap.asks.empty() && tape_snap.empty()) {
            Elements rows = { ctrl_row };
            for (auto& e : errors) rows.push_back(e);
            rows.push_back(separator());
            rows.push_back(text(running ? "  waiting for data…" : "  (no data)")
                           | dim | flex);
            return vbox(std::move(rows));
        }

        Elements top_rows = { ctrl_row };
        for (auto& e : errors) top_rows.push_back(e);
        top_rows.push_back(separator());

        auto body = hbox({
            vbox({ text(" Trades Tape ") | bold | color(Color::Cyan),
                   separator(),
                   render_tape(tape_snap),
                 }) | flex,
            separator(),
            vbox({ text(" Order Book ") | bold | color(Color::Cyan),
                   separator(),
                   render_book(snap),
                 }) | flex,
        }) | flex;

        top_rows.push_back(body);
        return vbox(std::move(top_rows));
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
