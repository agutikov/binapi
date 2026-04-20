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
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

#include <csignal>
#include <cstdlib>

namespace {

/// Best-effort crash signal handler: log the signal and flush the file
/// sink so the tail of the log survives. Then restore the default
/// handler and re-raise, so the process still dumps core / exits with
/// the expected status.
///
/// spdlog isn't strictly async-signal-safe (it takes a mutex on the
/// sink), but for a developer-facing demo the flush is worth the small
/// risk of deadlocking a process that's already crashing.
void crash_handler(int sig)
{
    const char* name = "unknown";
    switch (sig) {
        case SIGSEGV: name = "SIGSEGV"; break;
        case SIGABRT: name = "SIGABRT"; break;
        case SIGFPE:  name = "SIGFPE";  break;
        case SIGBUS:  name = "SIGBUS";  break;
        case SIGILL:  name = "SIGILL";  break;
        default: break;
    }
    spdlog::critical("crash_handler: caught {} (signum={})", name, sig);
    spdlog::apply_all([](std::shared_ptr<spdlog::logger> l) {
        if (l) l->flush();
    });
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

void install_crash_handlers()
{
    for (int s : { SIGSEGV, SIGABRT, SIGFPE, SIGBUS, SIGILL }) {
        std::signal(s, crash_handler);
    }
}

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
    std::string log_file;
    std::string log_level = "info";
    cli.add_flag("-l,--live,--prod", live, "Use production endpoints (default: testnet)");
    cli.add_option("-L,--log-file", log_file,
                   "Log to this file. If omitted, logging is fully silenced "
                   "(null sink) so terminal output isn't corrupted.");
    cli.add_option("-F,--file-loglevel", log_level,
                   "File log level: trace/debug/info/warn/error/critical/off")
        ->check(CLI::IsMember({ "trace", "debug", "info", "warn",
                                "error", "critical", "off" }))
        ->capture_default_str();
    CLI11_PARSE(cli, argc, argv);

    // spdlog routing. The TUI owns the terminal, so direct stdout/stderr
    // logging would corrupt the screen. Two modes:
    //
    //   * with `--log-file PATH` → every log line lands in PATH (sync
    //     basic_file_sink; does **not** add a thread), so the file is a
    //     clean debugging channel that includes anything binapi2_fapi /
    //     secret_provider / our worker itself logs.
    //   * without → default logger is a `null_sink`, level is OFF, and
    //     the UI is silent. Status still flows through app_state to
    //     the status bar.
    if (!log_file.empty()) {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            log_file, /*truncate=*/true);
        sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [t:%t] %v");
        auto logger = std::make_shared<spdlog::logger>("ui", sink);
        const auto lvl = spdlog::level::from_str(log_level);
        logger->set_level(lvl);
        // Flush on every line, even trace, so the tail of the log
        // survives an abrupt crash. Demo binary, not perf-critical.
        logger->flush_on(spdlog::level::trace);
        spdlog::set_default_logger(logger);
        spdlog::set_level(lvl);
        spdlog::info("demo-ui starting (live={}, file={}, level={})",
                     live, log_file, log_level);
        install_crash_handlers();
        spdlog::debug("crash handlers installed");
    } else {
        spdlog::set_default_logger(
            std::make_shared<spdlog::logger>("ui",
                std::make_shared<spdlog::sinks::null_sink_mt>()));
        spdlog::set_level(spdlog::level::off);
    }

    // ── shared state + worker thread ──────────────────────────────────
    spdlog::debug("main: constructing app_state");
    demo_ui::app_state state;
    state.use_testnet = !live;

    spdlog::debug("main: constructing ScreenInteractive::Fullscreen");
    auto screen = ftxui::ScreenInteractive::Fullscreen();

    spdlog::debug("main: constructing worker");
    demo_ui::worker wrk(state, screen, make_initial_config(live));

    spdlog::debug("main: starting worker");
    wrk.start();

    // ── tab layout ────────────────────────────────────────────────────
    using namespace ftxui;

    spdlog::debug("main: building component tree");

    int tab_index = 0;
    const std::vector<std::string> tab_titles = {
        "REST", "WS API", "Streams", "Order Book"
    };
    auto tab_toggle = Toggle(&tab_titles, &tab_index);

    spdlog::debug("main: building rest tab");
    auto rest_tab    = make_rest_view     (state, wrk);
    spdlog::debug("main: building ws tab");
    auto ws_tab      = make_ws_view       (state, wrk);
    spdlog::debug("main: building streams tab");
    auto streams_tab = make_streams_view  (state, wrk);
    spdlog::debug("main: building order book tab");
    auto book_tab    = make_orderbook_view(state, wrk);

    auto tab_content = Container::Tab(
        { rest_tab.component, ws_tab.component,
          streams_tab.component, book_tab.component },
        &tab_index);

    auto status_bar = make_status_bar(state);

    // `status_bar` is a non-interactive Renderer — including it in the
    // Container::Vertical would put it in the focus chain and swallow
    // keyboard events. Keep only the focusable children in `root` and
    // draw the status bar from the outer layout's vbox directly.
    auto root = Container::Vertical({
        tab_toggle,
        tab_content,
    });

    // Bottom keybar: asks the active tab for its chips each render, joins
    // them with `│` separators, always appends the global `Tab` / `q`
    // reminders at the far right.
    auto keybar = Renderer([&] {
        std::vector<Element> chips;
        switch (tab_index) {
            case 0: if (rest_tab.hints)    chips = rest_tab.hints();    break;
            case 1: if (ws_tab.hints)      chips = ws_tab.hints();      break;
            case 2: if (streams_tab.hints) chips = streams_tab.hints(); break;
            case 3: if (book_tab.hints)    chips = book_tab.hints();    break;
            default: break;
        }
        Elements row;
        row.push_back(text(" "));
        for (std::size_t i = 0; i < chips.size(); ++i) {
            if (i) row.push_back(text("   "));
            row.push_back(chips[i]);
        }
        row.push_back(filler());
        return hbox(std::move(row))
               | bgcolor(Color::GrayDark) | color(Color::White);
    });

    auto layout = Renderer(root, [&] {
        return vbox({
                   status_bar->Render(),
                   tab_toggle->Render() | center,
                   separator(),
                   tab_content->Render() | flex,
                   keybar->Render(),
               });
    });

    // Quit on 'q' (in addition to FTXUI's Ctrl-C handling).
    auto layout_with_quit = CatchEvent(layout, [&](Event e) {
        if (e == Event::Character('q')) {
            spdlog::info("main: 'q' pressed, exiting screen loop");
            screen.Exit();
            return true;
        }
        return false;
    });

    spdlog::info("main: entering screen.Loop");
    screen.Loop(layout_with_quit);
    spdlog::info("main: screen.Loop returned");

    // ── shutdown ──────────────────────────────────────────────────────
    spdlog::info("main: calling wrk.stop()");
    wrk.stop();
    spdlog::info("main: wrk.stop() returned; about to destruct locals");
    spdlog::apply_all([](std::shared_ptr<spdlog::logger> l) {
        if (l) l->flush();
    });
    return 0;
}
