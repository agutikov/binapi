// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-async-demo-ui — FTXUI-based interactive demonstration client
// for the binapi2 fapi library.
//
// Architecture (see also docs/binapi2/plans/async_demo_ui.md):
//
//   * The main thread runs the FTXUI ScreenInteractive event loop.
//   * A `worker` owns an asio::io_context on a background thread; all binapi
//     coroutines (`co_spawn`) run there.
//   * Worker tasks mutate `app_state` (atomics + a small mutex for strings)
//     and call `worker::notify_ui()` to wake the screen for re-render.
//
// Step 0 only renders the layout and reflects the credential-loading state
// in the status bar. Commands are wired in subsequent steps.

#include "app_state.hpp"
#include "worker.hpp"
#include "views/views.hpp"

#include <binapi2/fapi/config.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <CLI/CLI.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

namespace {

binapi2::fapi::config make_initial_config(bool live)
{
    if (live) {
        binapi2::fapi::config cfg;
        cfg.rest_host = "fapi.binance.com";
        cfg.websocket_api_host = "ws-fapi.binance.com";
        cfg.websocket_api_target = "/ws-fapi/v1";
        cfg.stream_host = "fstream.binance.com";
        return cfg;
    }
    return binapi2::fapi::config::testnet_config();
}

} // namespace

int main(int argc, char* argv[])
{
    // ── command-line ──────────────────────────────────────────────────
    CLI::App cli{ "binapi2-fapi-async-demo-ui — FTXUI demonstration client" };
    bool live = false;
    cli.add_flag("-l,--live,--prod", live, "Use production endpoints (default: testnet)");
    CLI11_PARSE(cli, argc, argv);

    // Send spdlog to /dev/null while the TUI owns the terminal — otherwise
    // logger output corrupts the screen. Real status flows through app_state.
    spdlog::set_default_logger(
        std::make_shared<spdlog::logger>("ui",
            std::make_shared<spdlog::sinks::null_sink_mt>()));
    spdlog::set_level(spdlog::level::off);

    // ── shared state + worker thread ──────────────────────────────────
    demo_ui::app_state state;
    state.use_testnet = !live;

    auto screen = ftxui::ScreenInteractive::Fullscreen();
    demo_ui::worker wrk(state, screen, make_initial_config(live));
    wrk.start();

    // ── tab layout ────────────────────────────────────────────────────
    using namespace ftxui;

    int tab_index = 0;
    const std::vector<std::string> tab_titles = {
        "REST", "WS API", "Streams", "Order Book"
    };
    auto tab_toggle = Toggle(&tab_titles, &tab_index);

    auto rest_tab    = make_rest_view     (state, wrk);
    auto ws_tab      = make_ws_view       (state, wrk);
    auto streams_tab = make_streams_view  (state, wrk);
    auto book_tab    = make_orderbook_view(state, wrk);

    auto tab_content = Container::Tab(
        { rest_tab, ws_tab, streams_tab, book_tab },
        &tab_index);

    auto status_bar = make_status_bar(state);

    auto root = Container::Vertical({
        status_bar,
        tab_toggle,
        tab_content,
    });

    auto layout = Renderer(root, [&] {
        return vbox({
                   status_bar->Render(),
                   tab_toggle->Render() | center,
                   separator(),
                   tab_content->Render() | flex,
               });
    });

    // Quit on 'q' (in addition to FTXUI's Ctrl-C handling).
    auto layout_with_quit = CatchEvent(layout, [&](Event e) {
        if (e == Event::Character('q')) {
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(layout_with_quit);

    // ── shutdown ──────────────────────────────────────────────────────
    wrk.stop();
    return 0;
}
