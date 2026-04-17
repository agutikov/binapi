// SPDX-License-Identifier: Apache-2.0
//
// Streams tab — step 3.
//
// Subscription selector (bookTicker for now), symbol input, Start/Stop
// buttons, a counter/status line, and a scrollable JSON event list
// showing the last N events from the ring buffer.

#include "../app_state.hpp"
#include "../streams/stream_capture.hpp"
#include "../streams/stream_capture_sink.hpp"
#include "../worker.hpp"
#include "views.hpp"

#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/fapi/types/enums.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;
namespace lib   = binapi2::demo;
namespace types = binapi2::fapi::types;

namespace {

// ── Stream subscriptions available in step 3 ──────────────────────

struct stream_def
{
    const char* name;
    const char* description;
    bool needs_symbol;
};

constexpr stream_def streams[] = {
    { "bookTicker",     "Individual book ticker",       true  },
    { "aggTrade",       "Aggregate trade",              true  },
    { "markPrice",      "Mark price",                   true  },
    { "ticker",         "24hr ticker",                  true  },
    { "miniTicker",     "Mini ticker",                  true  },
    { "!bookTicker",    "All book tickers",             false },
    { "!ticker",        "All 24hr tickers",             false },
    { "!miniTicker",    "All mini tickers",             false },
};

// ── Free-function coroutines ──────────────────────────────────────

boost::cobalt::task<void>
book_ticker_stream(worker& wrk,
                   std::shared_ptr<stream_capture_sink> sink,
                   std::shared_ptr<stream_capture> cap,
                   std::string symbol)
{
    spdlog::info("streams: starting bookTicker {}", symbol);
    types::book_ticker_subscription sub;
    sub.symbol = symbol;
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
}

boost::cobalt::task<void>
agg_trade_stream(worker& wrk,
                 std::shared_ptr<stream_capture_sink> sink,
                 std::shared_ptr<stream_capture> cap,
                 std::string symbol)
{
    spdlog::info("streams: starting aggTrade {}", symbol);
    types::aggregate_trade_subscription sub;
    sub.symbol = symbol;
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
}

boost::cobalt::task<void>
mark_price_stream(worker& wrk,
                  std::shared_ptr<stream_capture_sink> sink,
                  std::shared_ptr<stream_capture> cap,
                  std::string symbol)
{
    spdlog::info("streams: starting markPrice {}", symbol);
    types::mark_price_subscription sub;
    sub.symbol = symbol;
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
}

boost::cobalt::task<void>
ticker_stream(worker& wrk,
              std::shared_ptr<stream_capture_sink> sink,
              std::shared_ptr<stream_capture> cap,
              std::string symbol)
{
    spdlog::info("streams: starting ticker {}", symbol);
    types::ticker_subscription sub;
    sub.symbol = symbol;
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
}

boost::cobalt::task<void>
mini_ticker_stream(worker& wrk,
                   std::shared_ptr<stream_capture_sink> sink,
                   std::shared_ptr<stream_capture> cap,
                   std::string symbol)
{
    spdlog::info("streams: starting miniTicker {}", symbol);
    types::mini_ticker_subscription sub;
    sub.symbol = symbol;
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
}

boost::cobalt::task<void>
all_book_ticker_stream(worker& wrk,
                       std::shared_ptr<stream_capture_sink> sink,
                       std::shared_ptr<stream_capture> cap)
{
    spdlog::info("streams: starting !bookTicker (all)");
    co_await lib::exec_stream(wrk.api(), types::all_book_ticker_subscription{}, *sink, &cap->stop);
}

boost::cobalt::task<void>
all_ticker_stream(worker& wrk,
                  std::shared_ptr<stream_capture_sink> sink,
                  std::shared_ptr<stream_capture> cap)
{
    spdlog::info("streams: starting !ticker (all)");
    co_await lib::exec_stream(wrk.api(), types::all_market_ticker_subscription{}, *sink, &cap->stop);
}

boost::cobalt::task<void>
all_mini_ticker_stream(worker& wrk,
                       std::shared_ptr<stream_capture_sink> sink,
                       std::shared_ptr<stream_capture> cap)
{
    spdlog::info("streams: starting !miniTicker (all)");
    co_await lib::exec_stream(wrk.api(), types::all_market_mini_ticker_subscription{}, *sink, &cap->stop);
}

// ── Dispatch ──────────────────────────────────────────────────────

void start_stream(int stream_index,
                  const std::string& symbol,
                  worker& wrk,
                  app_state& state,
                  std::shared_ptr<stream_capture> cap)
{
    if (cap->running) return;

    // Reset capture.
    cap->stop = false;
    cap->total_events = 0;
    cap->errors = 0;
    {
        std::lock_guard lk(cap->mtx);
        cap->ring.clear();
        cap->error.clear();
    }

    auto sink = std::make_shared<stream_capture_sink>(cap, wrk, state);

    auto spawn = [&](auto coro) {
        boost::cobalt::spawn(wrk.io().get_executor(), std::move(coro),
                             boost::asio::use_future);
    };

    switch (stream_index) {
        case 0: spawn(book_ticker_stream(wrk, std::move(sink), cap, symbol));    break;
        case 1: spawn(agg_trade_stream(wrk, std::move(sink), cap, symbol));      break;
        case 2: spawn(mark_price_stream(wrk, std::move(sink), cap, symbol));     break;
        case 3: spawn(ticker_stream(wrk, std::move(sink), cap, symbol));         break;
        case 4: spawn(mini_ticker_stream(wrk, std::move(sink), cap, symbol));    break;
        case 5: spawn(all_book_ticker_stream(wrk, std::move(sink), cap));        break;
        case 6: spawn(all_ticker_stream(wrk, std::move(sink), cap));             break;
        case 7: spawn(all_mini_ticker_stream(wrk, std::move(sink), cap));        break;
        default: spdlog::warn("streams: unknown index {}", stream_index); break;
    }
}

} // namespace

Component make_streams_view(app_state& state, worker& wrk)
{
    // Per-stream captures — each stream type gets its own capture so
    // switching in the menu shows that stream's data.
    constexpr auto num_streams = sizeof(streams) / sizeof(streams[0]);
    auto caps = std::make_shared<std::vector<std::shared_ptr<stream_capture>>>();
    caps->reserve(num_streams);
    for (std::size_t i = 0; i < num_streams; ++i)
        caps->emplace_back(std::make_shared<stream_capture>());

    auto stream_titles = std::make_shared<std::vector<std::string>>();
    for (const auto& s : streams)
        stream_titles->emplace_back(s.name);
    auto stream_index = std::make_shared<int>(0);
    auto symbol = std::make_shared<std::string>("BTCUSDT");

    auto stream_menu = Menu(stream_titles.get(), stream_index.get());

    // Enter = toggle start/stop for the selected stream.
    auto menu_with_keys = CatchEvent(stream_menu,
        [&wrk, &state, caps, stream_index, symbol](Event e) {
            if (e == Event::Return) {
                auto idx = static_cast<std::size_t>(*stream_index);
                auto& cap = (*caps)[idx];
                if (cap->running) {
                    cap->stop = true;
                } else {
                    start_stream(*stream_index, *symbol, wrk, state, cap);
                }
                return true;
            }
            return false;
        });

    auto symbol_input = Input(symbol.get(), "symbol");
    auto symbol_maybe = symbol_input
        | Maybe([stream_index] {
              return streams[static_cast<std::size_t>(*stream_index)].needs_symbol;
          });

    auto scroll = std::make_shared<float>(0.f);
    constexpr float page_step = 0.15f;

    auto root = Container::Horizontal({
        menu_with_keys,
        symbol_maybe,
    });

    auto view = Renderer(root, [menu_with_keys, symbol_maybe,
                                stream_titles, stream_index, symbol, caps, scroll] {
        const auto idx = static_cast<std::size_t>(*stream_index);
        const auto& def = streams[idx];
        const auto& cap = (*caps)[idx];
        const bool running = cap->running.load();
        const auto total = cap->total_events.load();
        const auto errs = cap->errors.load();

        // Copy ring snapshot under lock.
        std::deque<std::string> ring_snap;
        std::string err_copy;
        {
            std::lock_guard lk(cap->mtx);
            ring_snap = cap->ring;
            err_copy = cap->error;
        }

        // ── controls ──────────────────────────────────────────────
        Elements ctrl = {
            text(def.name) | bold,
            text("  "),
            text(def.description) | dim,
        };
        if (def.needs_symbol) {
            ctrl.push_back(text("  "));
            ctrl.push_back(symbol_maybe->Render() | size(WIDTH, EQUAL, 12) | border);
        }

        // ── status line ───────────────────────────────────────────
        auto status = hbox({
            text(running ? " RUNNING " : " STOPPED ") | bold
                | (running ? bgcolor(Color::Green) | color(Color::Black)
                           : bgcolor(Color::GrayDark) | color(Color::White)),
            text("  events: " + std::to_string(total)),
            text("  errors: " + std::to_string(errs))
                | (errs > 0 ? color(Color::Red) : dim),
            filler(),
            text("Enter = start/stop  PgUp/PgDn = scroll") | dim,
        });

        if (!err_copy.empty()) {
            status = vbox({ status,
                            text("  ERROR: " + err_copy) | color(Color::Red) });
        }

        // ── event list ────────────────────────────────────────────
        Elements rows;
        for (auto it = ring_snap.rbegin(); it != ring_snap.rend(); ++it)
            rows.push_back(paragraph(*it));
        if (rows.empty())
            rows.push_back(text(running ? "  waiting for events…" : "  (no events)") | dim);

        auto event_list = vbox(std::move(rows))
            | focusPositionRelative(0.f, *scroll)
            | yframe
            | vscroll_indicator
            | flex;

        return vbox({
            hbox({
                vbox({ text("Streams") | bold, separator(),
                       menu_with_keys->Render() | flex })
                    | size(WIDTH, EQUAL, 18),
                separator(),
                vbox({
                    hbox(std::move(ctrl)),
                    separator(),
                    status,
                    separator(),
                    event_list,
                }) | flex,
            }),
        });
    });

    return CatchEvent(view, [scroll](Event e) {
        if (e == Event::PageDown) { *scroll = std::min(1.f, *scroll + page_step); return true; }
        if (e == Event::PageUp)   { *scroll = std::max(0.f, *scroll - page_step); return true; }
        return false;
    });
}

} // namespace demo_ui
