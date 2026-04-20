// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API tab — a handful of WS-API commands that connect,
// execute one JSON-RPC call, and disconnect. Same layout as the REST
// tab: command menu on the left, request/response panes on the right.

#include "../app_state.hpp"
#include "../util/capture_sink.hpp"
#include "../util/request_capture.hpp"
#include "../worker.hpp"
#include "response_pane.hpp"
#include "views.hpp"

#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;
namespace lib   = binapi2::demo;
namespace types = binapi2::fapi::types;

namespace {

struct ws_command
{
    const char* name;
    const char* description;
    bool needs_symbol;
    bool needs_auth;     // signed (logon) vs public
};

constexpr ws_command commands[] = {
    { "ws-book-ticker",    "Book ticker via WS API [symbol]",        true,  false },
    { "ws-price-ticker",   "Price ticker via WS API [symbol]",       true,  false },
    { "ws-logon",          "Session logon (auth)",                    false, true  },
    { "ws-account-status", "Account status (auth)",                   false, true  },
    { "ws-account-balance","Account balance (auth)",                  false, true  },
};

// ── Coroutines ────────────────────────────────────────────────────

boost::cobalt::task<void>
ws_book_ticker_coro(worker& wrk,
                    std::shared_ptr<capture_sink> sink,
                    std::shared_ptr<request_capture> cap,
                    std::string symbol)
{
    active_capture_guard guard(wrk, cap.get());
    if (symbol.empty()) {
        co_await lib::exec_ws_public(wrk.api(), types::websocket_api_book_tickers_request_t{}, *sink);
    } else {
        types::websocket_api_book_ticker_request_t req;
        req.symbol = symbol;
        co_await lib::exec_ws_public(wrk.api(), std::move(req), *sink);
    }
}

boost::cobalt::task<void>
ws_price_ticker_coro(worker& wrk,
                     std::shared_ptr<capture_sink> sink,
                     std::shared_ptr<request_capture> cap,
                     std::string symbol)
{
    active_capture_guard guard(wrk, cap.get());
    if (symbol.empty()) {
        co_await lib::exec_ws_public(wrk.api(), types::websocket_api_price_tickers_request_t{}, *sink);
    } else {
        types::websocket_api_price_ticker_request_t req;
        req.symbol = symbol;
        co_await lib::exec_ws_public(wrk.api(), std::move(req), *sink);
    }
}

boost::cobalt::task<void>
ws_logon_coro(worker& wrk,
              std::shared_ptr<capture_sink> sink,
              std::shared_ptr<request_capture> cap)
{
    active_capture_guard guard(wrk, cap.get());
    auto ws = co_await wrk.api().create_ws_api_client();
    if (!ws) { sink->on_error(ws.err); sink->on_done(1); co_return; }
    if (auto c = co_await (*ws)->async_connect(); !c) { sink->on_error(c.err); sink->on_done(1); co_return; }
    auto r = co_await (*ws)->async_session_logon();
    if (!r) { sink->on_error(r.err); co_await (*ws)->async_close(); sink->on_done(1); co_return; }
    sink->on_info("session logon ok, status=" + std::to_string(r->status));
    if (r->result) {
        if (auto j = glz::write<glz::opts{ .prettify = true }>(*r->result); j)
            sink->on_response_json(*j);
    }
    co_await (*ws)->async_close();
    sink->on_done(0);
}

boost::cobalt::task<void>
ws_account_status_coro(worker& wrk,
                       std::shared_ptr<capture_sink> sink,
                       std::shared_ptr<request_capture> cap)
{
    active_capture_guard guard(wrk, cap.get());
    co_await lib::exec_ws_signed(wrk.api(), types::ws_account_status_request_t{}, *sink);
}

boost::cobalt::task<void>
ws_account_balance_coro(worker& wrk,
                        std::shared_ptr<capture_sink> sink,
                        std::shared_ptr<request_capture> cap)
{
    active_capture_guard guard(wrk, cap.get());
    co_await lib::exec_ws_signed(wrk.api(), types::ws_account_balance_request_t{}, *sink);
}

// ── Dispatch ──────────────────────────────────────────────────────

void run_ws_command(int cmd_index,
                    const std::string& symbol,
                    worker& wrk,
                    app_state& state,
                    std::shared_ptr<request_capture> cap)
{
    spdlog::info("ws_view: run cmd_index={} symbol='{}'", cmd_index, symbol);

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

    auto spawn = [&](auto coro) {
        boost::cobalt::spawn(wrk.io().get_executor(), std::move(coro),
                             boost::asio::use_future);
    };

    switch (cmd_index) {
        case 0: spawn(ws_book_ticker_coro(wrk, std::move(sink), cap, symbol));   break;
        case 1: spawn(ws_price_ticker_coro(wrk, std::move(sink), cap, symbol));  break;
        case 2: spawn(ws_logon_coro(wrk, std::move(sink), cap));                 break;
        case 3: spawn(ws_account_status_coro(wrk, std::move(sink), cap));        break;
        case 4: spawn(ws_account_balance_coro(wrk, std::move(sink), cap));       break;
        default: break;
    }
}

} // namespace

tab_view make_ws_view(app_state& state, worker& wrk)
{
    constexpr auto num_cmds = sizeof(commands) / sizeof(commands[0]);
    auto caps = std::make_shared<std::vector<std::shared_ptr<request_capture>>>();
    caps->reserve(num_cmds);
    for (std::size_t i = 0; i < num_cmds; ++i)
        caps->emplace_back(std::make_shared<request_capture>());

    auto cmd_titles = std::make_shared<std::vector<std::string>>();
    for (const auto& c : commands)
        cmd_titles->emplace_back(c.name);
    auto cmd_index = std::make_shared<int>(0);
    auto symbol = std::make_shared<std::string>("BTCUSDT");

    auto cmd_menu = Menu(cmd_titles.get(), cmd_index.get());
    auto cmd_menu_with_enter = CatchEvent(cmd_menu,
        [&wrk, &state, caps, cmd_index, symbol](Event e) {
            if (e == Event::Return) {
                run_ws_command(*cmd_index, *symbol, wrk, state,
                              (*caps)[static_cast<std::size_t>(*cmd_index)]);
                return true;
            }
            return false;
        });

    auto symbol_input = Input(symbol.get(), "symbol");
    auto symbol_maybe = symbol_input
        | Maybe([cmd_index] {
              return commands[static_cast<std::size_t>(*cmd_index)].needs_symbol;
          });

    auto rr_pane = make_request_response_pane([caps, cmd_index]() {
        return (*caps)[static_cast<std::size_t>(*cmd_index)];
    });

    auto root = Container::Horizontal({
        cmd_menu_with_enter,
        symbol_maybe,
        rr_pane,
    });

    auto component = Renderer(root, [cmd_menu_with_enter, symbol_maybe, rr_pane,
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
            text(cmd.needs_auth ? "(auth)" : "(public)") | dim,
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

        return hbox({
            vbox({ text("WS API") | bold, separator(),
                   cmd_menu_with_enter->Render() | flex })
                | size(WIDTH, EQUAL, 22),
            separator(),
            vbox(std::move(mid_rows)) | size(WIDTH, EQUAL, 42),
            separator(),
            rr_pane->Render() | flex,
        });
    });

    auto hints = []() -> std::vector<Element> {
        return {
            key_chip("↑↓", "select"),
            key_chip("Enter", "run"),
            key_chip("→/←", "zones"),
            key_chip("Tab", "cycle tabs"),
            key_chip("q", "quit"),
        };
    };
    return tab_view{ component, std::move(hints) };
}

} // namespace demo_ui
