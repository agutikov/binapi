// SPDX-License-Identifier: Apache-2.0
//
// REST tab — step 1 expanded.
//
// Four commands hand-coded into a menu, a single symbol Input field
// for the ones that take a symbol, a Run button that dispatches to the
// selected command, and a stacked Request/Response display with
// Raw/JSON/Tree sub-tabs per half.
//
// Commands are deliberately picked to exercise the full pipeline:
//
//   * `ping`           — smallest possible response (`{}`)
//   * `server-time`    — one-field response
//   * `ticker-24hr`    — medium, requires a symbol param
//   * `exchange-info`  — huge, ~200 symbols × many fields, nested —
//                        stresses json_tree's depth cap and the
//                        render path
//
// Step 4 will replace this hand-coding with a proper registry of ~80
// REST commands + per-command parameter forms; for now the pattern is:
// when a new command is added, add a `run_<cmd>` free function (free
// functions are required — see feedback_cobalt_lambda_lifetime.md),
// add an entry to `commands[]`, and (if it takes a symbol) toggle the
// `needs_symbol` flag.
//
// ── Lifetime notes ───────────────────────────────────────────────
//
//   * FTXUI widgets like `Menu`, `Toggle`, `Input` store **raw
//     pointers** into caller-owned models. Every model lives inside a
//     `shared_ptr` that's **explicitly captured** in the outer
//     `Renderer` lambda so the pointers outlive `make_rest_view`.
//
//   * Cobalt task bodies are **free functions**, never local lambdas.
//     A local lambda captured by value gets destroyed when the
//     spawning function returns, but the coroutine frame holds a
//     pointer back into the lambda's captures — use-after-free on
//     the next resumption. (`feedback_cobalt_lambda_lifetime.md`.)

#include "../app_state.hpp"
#include "../util/capture_sink.hpp"
#include "../util/json_tree.hpp"
#include "../util/request_capture.hpp"
#include "../worker.hpp"
#include "views.hpp"

#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/fapi/types/market_data.hpp>
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

// ── Rendering helpers ─────────────────────────────────────────────

constexpr float page_step = 0.15f;

/// Pure-display scrollable text renderer. Uses a SINGLE `paragraph()`
/// element for the entire content — no per-line split, no truncation.
/// FTXUI's `paragraph()` handles newlines + word-wrap internally as
/// one DOM node, so even a 1.6 MB exchange-info response renders
/// without freezing.
///
/// The content string is cached: `content_fn()` is called every frame
/// (cheap mutex-locked copy), but the `paragraph()` Element is only
/// rebuilt when the string actually changes.
Component make_scrollable_text(std::function<std::string()> content_fn,
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

/// Pure-display scrollable tree renderer. The tree Element is cached
/// and only rebuilt when the underlying `glz::generic` pointer changes
/// (i.e. a new response arrived).
Component make_scrollable_tree(std::function<std::shared_ptr<glz::generic>()> tree_fn,
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

// ── JSON pre-fill ─────────────────────────────────────────────────

/// Pre-serialize a typed request struct into the capture's request
/// side. Runs on the main thread at Run-click time, so the top half
/// is populated immediately even before the network call starts.
template<typename Request>
void prefill_request_json(request_capture& cap, const Request& req)
{
    auto j = glz::write<glz::opts{ .prettify = true }>(req);
    if (!j) return;
    std::lock_guard lk(cap.mtx);
    fill_pretty_and_parsed(cap.request, *j);
}

// ── Coroutine task bodies ─────────────────────────────────────────
//
// Free functions only. Each takes the state it needs by value /
// shared_ptr so the coroutine frame owns it for the full lifetime of
// the async op.

boost::cobalt::task<void>
ping_coro(worker& wrk,
          std::shared_ptr<capture_sink> sink,
          std::shared_ptr<request_capture> cap)
{
    spdlog::debug("rest_view: ping coroutine entered");
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    co_await lib::exec_market_data(*rest, types::ping_request_t{}, *sink);
    spdlog::debug("rest_view: ping coroutine completed");
}

boost::cobalt::task<void>
time_coro(worker& wrk,
          std::shared_ptr<capture_sink> sink,
          std::shared_ptr<request_capture> cap)
{
    spdlog::debug("rest_view: time coroutine entered");
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    co_await lib::exec_market_data(*rest, types::server_time_request_t{}, *sink);
    spdlog::debug("rest_view: time coroutine completed");
}

boost::cobalt::task<void>
exchange_info_coro(worker& wrk,
                   std::shared_ptr<capture_sink> sink,
                   std::shared_ptr<request_capture> cap)
{
    spdlog::debug("rest_view: exchange-info coroutine entered");
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    co_await lib::exec_market_data(*rest, types::exchange_info_request_t{}, *sink);
    spdlog::debug("rest_view: exchange-info coroutine completed");
}

boost::cobalt::task<void>
ticker_24hr_coro(worker& wrk,
                 std::shared_ptr<capture_sink> sink,
                 std::shared_ptr<request_capture> cap,
                 std::string symbol)
{
    spdlog::debug("rest_view: ticker-24hr coroutine entered, symbol={}", symbol);
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    types::ticker_24hr_request_t req;
    req.symbol = symbol;
    co_await lib::exec_market_data(*rest, std::move(req), *sink);
    spdlog::debug("rest_view: ticker-24hr coroutine completed");
}

// ── Command registry ──────────────────────────────────────────────

struct ui_command
{
    const char* name;
    const char* description;
    bool needs_symbol;
};

constexpr ui_command commands[] = {
    { "ping",          "Test API connectivity (empty response)",   false },
    { "server-time",   "Server time (1 field)",                    false },
    { "exchange-info", "Exchange info (huge, ~200 symbols)",       false },
    { "ticker-24hr",   "24hr price-change stats (requires symbol)", true  },
};

// ── Per-command dispatch ──────────────────────────────────────────

/// Dispatch to the right coroutine based on `cmd_index`. Handles the
/// shared capture-reset + shared-sink-construction bookkeeping in one
/// place so each command's coroutine stays tiny.
void run_selected(int cmd_index,
                  const std::string& symbol,
                  worker& wrk,
                  app_state& state,
                  std::shared_ptr<request_capture> cap)
{
    spdlog::info("rest_view: run_selected cmd_index={} symbol='{}'",
                 cmd_index, symbol);

    // Reset the capture for a fresh run.
    {
        std::lock_guard lk(cap->mtx);
        cap->info_lines.clear();
        cap->request  = capture_side{};
        cap->response = capture_side{};
        cap->error_message.clear();
        cap->http_status = 0;
        cap->binance_code = 0;
    }
    cap->state.store(request_capture::idle);

    auto sink = std::make_shared<capture_sink>(cap, wrk, state);

    // Pre-fill the Request JSON side on the main thread so the top
    // half is populated immediately, before the coroutine even runs.
    switch (cmd_index) {
        case 0: prefill_request_json(*cap, types::ping_request_t{});         break;
        case 1: prefill_request_json(*cap, types::server_time_request_t{});  break;
        case 2: prefill_request_json(*cap, types::exchange_info_request_t{});break;
        case 3: {
            types::ticker_24hr_request_t req;
            req.symbol = symbol;
            prefill_request_json(*cap, req);
            break;
        }
        default: break;
    }

    // Spawn the right coroutine on the worker.
    switch (cmd_index) {
        case 0:
            boost::cobalt::spawn(
                wrk.io().get_executor(),
                ping_coro(wrk, std::move(sink), cap),
                boost::asio::use_future);
            break;
        case 1:
            boost::cobalt::spawn(
                wrk.io().get_executor(),
                time_coro(wrk, std::move(sink), cap),
                boost::asio::use_future);
            break;
        case 2:
            boost::cobalt::spawn(
                wrk.io().get_executor(),
                exchange_info_coro(wrk, std::move(sink), cap),
                boost::asio::use_future);
            break;
        case 3:
            boost::cobalt::spawn(
                wrk.io().get_executor(),
                ticker_24hr_coro(wrk, std::move(sink), cap, symbol),
                boost::asio::use_future);
            break;
        default:
            spdlog::warn("rest_view: unknown cmd_index {}", cmd_index);
            break;
    }
}

// ── Request / Response display ────────────────────────────────────

/// `get_cap_fn` returns the currently-selected capture (depends on
/// which command is highlighted in the menu). Called at render time.
using get_cap_fn = std::function<std::shared_ptr<request_capture>()>;

/// Build a Raw/JSON/Tree pane bound to one side of whatever capture
/// `get_cap` returns at render time. When the user switches commands
/// in the menu, the pane automatically shows that command's last
/// request/response.
Component make_side_pane(get_cap_fn get_cap,
                         bool is_request,
                         const char* title)
{
    auto sub_tab_titles = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{ "Raw", "JSON", "Tree" });
    auto sub_tab_index = std::make_shared<int>(0);

    // Per-tab scroll state. Index matches sub_tab_index: 0=Raw, 1=JSON, 2=Tree.
    auto scrolls = std::make_shared<std::array<std::shared_ptr<float>, 3>>(
        std::array<std::shared_ptr<float>, 3>{
            std::make_shared<float>(0.f),
            std::make_shared<float>(0.f),
            std::make_shared<float>(0.f),
        });

    // Raw view
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

    // JSON view
    auto json_view = make_scrollable_text([get_cap, is_request]() -> std::string {
        auto cap = get_cap();
        std::lock_guard lk(cap->mtx);
        const auto& side = is_request ? cap->request : cap->response;
        return side.pretty_json.empty()
            ? std::string{ "(no JSON yet)" }
            : side.pretty_json;
    }, (*scrolls)[1]);

    // Tree view
    auto tree_view = make_scrollable_tree([get_cap, is_request]() -> std::shared_ptr<glz::generic> {
        auto cap = get_cap();
        std::lock_guard lk(cap->mtx);
        return is_request ? cap->request.parsed_json : cap->response.parsed_json;
    }, (*scrolls)[2]);

    auto tab_toggle = Toggle(sub_tab_titles.get(), sub_tab_index.get());
    auto tab_content = Container::Tab(
        { raw_view, json_view, tree_view },
        sub_tab_index.get());

    // Only tab_toggle is in the focus chain — the tab content
    // renderers are non-focusable so arrow keys navigate normally.
    // PageUp/PageDown are caught HERE and routed to the active tab's
    // scroll state.
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

/// Stacked Request / Response display: top half = request, bottom = response.
Component make_request_response_pane(get_cap_fn get_cap)
{
    auto request_pane  = make_side_pane(get_cap, /*is_request=*/true,  "REQUEST");
    auto response_pane = make_side_pane(get_cap, /*is_request=*/false, "RESPONSE");

    auto inner = Container::Vertical({ request_pane, response_pane });

    return Renderer(inner, [request_pane, response_pane] {
        // Exact 50/50 vertical split. `yflex_grow` alone doesn't work
        // because FTXUI respects each child's `min_y`, giving more
        // space to whichever half has more content. Instead, compute
        // the available height from Terminal::Size() and assign
        // exactly half to each pane via `size(HEIGHT, EQUAL, half)`.
        //
        // Overhead subtracted: status bar (1) + tab toggle (1) +
        // separator (1) + title+toggle per half (2×2=4) + this
        // separator (1) = ~8 rows.
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

} // namespace

Component make_rest_view(app_state& state, worker& wrk)
{
    spdlog::debug("make_rest_view: entering");

    // Per-command captures — each command gets its own request_capture so
    // switching commands in the menu shows that command's last result.
    constexpr auto num_cmds = sizeof(commands) / sizeof(commands[0]);
    auto caps = std::make_shared<std::vector<std::shared_ptr<request_capture>>>();
    caps->reserve(num_cmds);
    for (std::size_t i = 0; i < num_cmds; ++i)
        caps->emplace_back(std::make_shared<request_capture>());

    // Command menu: hand-coded titles from the `commands[]` table.
    auto cmd_titles = std::make_shared<std::vector<std::string>>();
    for (const auto& c : commands)
        cmd_titles->emplace_back(c.name);
    auto cmd_index = std::make_shared<int>(0);

    // Symbol input (used by commands with `needs_symbol`).
    auto symbol = std::make_shared<std::string>("BTCUSDT");

    // Fire request on Enter in the command menu.
    auto cmd_menu = Menu(cmd_titles.get(), cmd_index.get());
    auto cmd_menu_with_enter = CatchEvent(cmd_menu, [&wrk, &state, caps, cmd_index, symbol](Event e) {
        if (e == Event::Return) {
            run_selected(*cmd_index, *symbol, wrk, state,
                         (*caps)[static_cast<std::size_t>(*cmd_index)]);
            return true;
        }
        return false;
    });

    // Symbol input — only focusable (and rendered) when the selected
    // command has `needs_symbol`. `Maybe` makes the component return
    // `Focusable() == false` when hidden, so Container::Horizontal
    // skips it during →/← navigation and focus jumps straight from
    // the command list to the response pane.
    auto symbol_input = Input(symbol.get(), "symbol");
    auto symbol_maybe = symbol_input
        | Maybe([cmd_index] {
              return commands[static_cast<std::size_t>(*cmd_index)].needs_symbol;
          });

    auto rr_pane = make_request_response_pane([caps, cmd_index]() {
        return (*caps)[static_cast<std::size_t>(*cmd_index)];
    });

    // Focus chain: cmd_menu → symbol_input (only when visible) → rr_pane.
    auto root = Container::Horizontal({
        cmd_menu_with_enter,
        symbol_maybe,
        rr_pane,
    });

    // Explicit capture of cmd_titles / cmd_index / symbol / cap keeps
    // the models alive for the lifetime of the returned component.
    // The middle column is **rendered** here (non-interactive text:
    // title, description, state, info) rather than wrapped in a
    // Renderer-with-child, so it never traps focus.
    return Renderer(root, [cmd_menu_with_enter, symbol_maybe, rr_pane,
                           cmd_titles, cmd_index, symbol, caps] {
        const auto idx = static_cast<std::size_t>(*cmd_index);
        const auto& cmd = commands[idx];
        const auto& cap = (*caps)[idx];

        const char* state_label = "";
        switch (cap->state.load()) {
            case request_capture::idle:    state_label = "idle"; break;
            case request_capture::running: state_label = "running…"; break;
            case request_capture::done:    state_label = "done"; break;
            case request_capture::failed:  state_label = "failed"; break;
        }
        std::string info_copy;
        {
            std::lock_guard lk(cap->mtx);
            info_copy = cap->info_lines;
        }

        Elements mid_rows = {
            text(cmd.name) | bold,
            text(cmd.description) | dim,
            separator(),
        };
        if (cmd.needs_symbol) {
            mid_rows.push_back(hbox({
                text("symbol: ") | dim,
                symbol_maybe->Render() | border | flex,
            }));
            mid_rows.push_back(separator());
        }
        mid_rows.push_back(hbox({ text("state: "), text(state_label) | bold }));
        mid_rows.push_back(separator());
        mid_rows.push_back(text("info:") | dim);
        mid_rows.push_back(text(info_copy.empty() ? std::string{ "(none)" } : info_copy));
        mid_rows.push_back(filler());
        mid_rows.push_back(text("Enter = run  ↑↓ = select  →/← = navigate") | dim);

        return hbox({
                   vbox({ text("Commands") | bold, separator(),
                          cmd_menu_with_enter->Render() | flex })
                       | size(WIDTH, EQUAL, 22),
                   separator(),
                   vbox(std::move(mid_rows)) | size(WIDTH, EQUAL, 42),
                   separator(),
                   rr_pane->Render() | flex,
               });
    });
}

} // namespace demo_ui
