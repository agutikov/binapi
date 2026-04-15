// SPDX-License-Identifier: Apache-2.0
//
// REST tab — step 1.
//
// One command (`ping`) selectable on the left, a Run button in the
// middle, and a stacked **Request / Response** display on the right.
// Each half (top = request, bottom = response) has three sub-tabs:
// Raw / JSON / Tree.
//
//   * Raw  → full HTTP message (request line + headers + body for the
//            request, status line + headers + body for the response).
//            Filled by `cfg.logger` via `worker::active_capture_`.
//   * JSON → typed struct serialized via glaze (the typed Request struct
//            for the top half; the typed Response struct from
//            `capture_sink::on_response_json` for the bottom half).
//   * Tree → glz::generic walker (depth-capped) over the same JSON.
//
// ── Lifetime note ─────────────────────────────────────────────────
// FTXUI widgets like `Menu` and `Toggle` store **raw pointers** into
// caller-owned vectors / ints (the "model"). Those models must outlive
// the widget — we hold each one in a `shared_ptr` and **explicitly
// capture it** in every lambda retained by the returned component tree.
// Same trap for cobalt task bodies: never wrap a coroutine body in a
// local lambda that captures shared_ptrs (see
// `feedback_cobalt_lambda_lifetime.md`); use a free function so the
// args live in the coroutine frame.

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

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;
namespace lib = binapi2::demo;

namespace {

/// Split a newline-delimited string into one `text()` element per line.
Elements split_lines(const std::string& src)
{
    Elements out;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= src.size(); ++i) {
        if (i == src.size() || src[i] == '\n') {
            out.push_back(text(src.substr(start, i - start)));
            start = i + 1;
        }
    }
    if (out.empty()) out.push_back(text(""));
    return out;
}

/// Pre-serialize a typed request struct into a `capture_side`'s
/// pretty/parsed JSON fields. Called on the main thread before
/// spawning, so the Request half is populated immediately on click
/// even if the network call hasn't started yet.
template<typename Request>
void prefill_request_json(request_capture& cap, const Request& req)
{
    auto j = glz::write<glz::opts{ .prettify = true }>(req);
    if (!j) return;
    std::lock_guard lk(cap.mtx);
    fill_pretty_and_parsed(cap.request, *j);
}

/// The ping coroutine itself. **Must be a free function (not a local
/// lambda)** so its arguments — in particular the `shared_ptr<capture_sink>`
/// — live inside the coroutine frame.
///
/// Uses the worker's **persistent** REST client (lazy-connected on first
/// call, then reused) instead of creating a fresh client per request.
boost::cobalt::task<void>
ping_coro(worker& wrk,
          std::shared_ptr<capture_sink> sink,
          std::shared_ptr<request_capture> cap)
{
    spdlog::debug("rest_view: ping coroutine entered");

    // Scope cfg.logger output to this request: the guard sets
    // worker::active_capture_ in its ctor and clears it in its dtor.
    active_capture_guard guard(wrk, cap.get());

    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) {
        // acquire_rest_client already emitted error + on_done via sink.
        spdlog::warn("rest_view: ping aborted — REST client unavailable");
        co_return;
    }

    co_await lib::exec_market_data(
        *rest, binapi2::fapi::types::ping_request_t{}, *sink);

    spdlog::debug("rest_view: ping coroutine completed");
}

/// Spawn a `ping` request on the worker.
void run_ping(worker& wrk, app_state& state, std::shared_ptr<request_capture> cap)
{
    spdlog::info("rest_view: run_ping clicked");

    // Reset capture under lock — fresh state for the new run.
    {
        std::lock_guard lk(cap->mtx);
        cap->info_lines.clear();
        cap->request = capture_side{};
        cap->response = capture_side{};
        cap->error_message.clear();
        cap->http_status = 0;
        cap->binance_code = 0;
    }
    cap->state.store(request_capture::idle);

    // Pre-fill the typed Request JSON so the top half shows something
    // immediately (ping is `{}` — boring but visible).
    prefill_request_json(*cap, binapi2::fapi::types::ping_request_t{});

    auto sink = std::make_shared<capture_sink>(cap, wrk, state);

    auto fut = boost::cobalt::spawn(
        wrk.io().get_executor(),
        ping_coro(wrk, std::move(sink), cap),
        boost::asio::use_future);
    (void)fut;  // detach
}

// ── Request / Response display ────────────────────────────────────

/// Build a Raw/JSON/Tree pane bound to one side of a `request_capture`
/// (either `cap->request` or `cap->response`, selected by `is_request`).
/// `title` labels the half ("REQUEST" / "RESPONSE").
Component make_side_pane(const std::shared_ptr<request_capture>& cap,
                         bool is_request,
                         const char* title)
{
    auto sub_tab_titles = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{ "Raw", "JSON", "Tree" });
    auto sub_tab_index = std::make_shared<int>(0);

    // Each sub-renderer takes a const-ref to whichever capture_side it
    // displays under cap->mtx, copies it out, and renders.
    auto get_side = [cap, is_request]() -> capture_side {
        std::lock_guard lk(cap->mtx);
        return is_request ? cap->request : cap->response;
        // Note: copies the strings + shared_ptr. parsed_json is shared,
        // not deep-copied, so the tree view sees a stable snapshot.
    };

    auto raw_view = Renderer([cap, get_side, is_request] {
        // For the response side, surface the typed error in Raw too.
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
        const auto side = get_side();
        std::string text_copy = side.raw;
        if (!error_msg.empty()) {
            if (!text_copy.empty()) text_copy += "\n\n";
            text_copy += error_msg;
        }
        if (text_copy.empty())
            text_copy = is_request ? "(no request yet)" : "(no response yet)";
        return vbox(split_lines(text_copy)) | yframe | flex;
    });

    auto json_view = Renderer([get_side, is_request] {
        const auto side = get_side();
        std::string copy = side.pretty_json.empty()
            ? std::string{ is_request ? "(no JSON yet)" : "(no JSON yet)" }
            : side.pretty_json;
        return vbox(split_lines(copy)) | yframe | flex;
    });

    auto tree_view = Renderer([get_side] {
        const auto side = get_side();
        return render_json_tree(side.parsed_json) | yframe | flex;
    });

    auto tab_toggle = Toggle(sub_tab_titles.get(), sub_tab_index.get());
    auto tab_content = Container::Tab(
        { raw_view, json_view, tree_view },
        sub_tab_index.get());

    auto inner = Container::Vertical({ tab_toggle, tab_content });

    return Renderer(inner, [tab_toggle, tab_content,
                            sub_tab_titles, sub_tab_index, title] {
        return vbox({
                   hbox({ text(title) | bold | color(Color::Cyan),
                          text("  "),
                          tab_toggle->Render() }),
                   separator(),
                   tab_content->Render() | flex,
               });
    });
}

/// Stacked Request / Response display: top half is the request side,
/// bottom half is the response side. Each half is its own
/// Raw/JSON/Tree component built by `make_side_pane`.
Component make_request_response_pane(const std::shared_ptr<request_capture>& cap)
{
    auto request_pane  = make_side_pane(cap, /*is_request=*/true,  "REQUEST");
    auto response_pane = make_side_pane(cap, /*is_request=*/false, "RESPONSE");

    auto inner = Container::Vertical({ request_pane, response_pane });

    return Renderer(inner, [request_pane, response_pane] {
        return vbox({
                   request_pane->Render() | flex,
                   separator(),
                   response_pane->Render() | flex,
               });
    });
}

} // namespace

Component make_rest_view(app_state& state, worker& wrk)
{
    spdlog::debug("make_rest_view: entering");
    // Per-view state held in shared_ptrs so the lambdas below extend
    // their lifetime past this function's return.
    auto cap = std::make_shared<request_capture>();
    auto cmd_titles = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{ "ping" });
    auto cmd_index = std::make_shared<int>(0);

    auto cmd_menu = Menu(cmd_titles.get(), cmd_index.get());

    auto run_button = Button("Run", [&wrk, &state, cap] {
        run_ping(wrk, state, cap);
    });

    auto params_pane = Renderer(run_button, [run_button, cap] {
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
        return vbox({
                   text("ping") | bold,
                   text("Test API connectivity") | dim,
                   separator(),
                   run_button->Render(),
                   separator(),
                   hbox({ text("state: "), text(state_label) | bold }),
                   separator(),
                   text("info:") | dim,
                   text(info_copy.empty() ? std::string{ "(none)" } : info_copy),
               });
    });

    auto rr_pane = make_request_response_pane(cap);

    auto root = Container::Horizontal({
        cmd_menu,
        params_pane,
        rr_pane,
    });

    // Explicit capture of cmd_titles / cmd_index / cap extends their
    // lifetime for as long as the returned component lives — see the
    // lifetime note at the top of this file.
    return Renderer(root, [cmd_menu, params_pane, rr_pane,
                           cmd_titles, cmd_index, cap] {
        return hbox({
                   vbox({ text("Commands") | bold, separator(),
                          cmd_menu->Render() | flex })
                       | size(WIDTH, EQUAL, 20),
                   separator(),
                   params_pane->Render() | size(WIDTH, EQUAL, 38),
                   separator(),
                   rr_pane->Render() | flex,
               });
    });
}

} // namespace demo_ui
