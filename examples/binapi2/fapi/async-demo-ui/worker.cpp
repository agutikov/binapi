// SPDX-License-Identifier: Apache-2.0

#include "worker.hpp"

#include <binapi2/fapi/secret_config.hpp>
#include <secret_provider/factory.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <ftxui/component/event.hpp>

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <future>

namespace demo_ui {

namespace {

/// Cobalt task that loads API credentials and updates `state` accordingly.
boost::cobalt::task<void>
load_credentials_task(app_state& state, binapi2::futures_usdm_api& api,
                      worker& wrk)
{
    // Resolve the same default the CLI uses: libsecret:demo unless overridden
    // via env var BINAPI_DEMO_SECRETS (read once at startup).
    const char* override_src = std::getenv("BINAPI_DEMO_SECRETS");
    std::string source = override_src && *override_src ? override_src : "libsecret:demo";

    auto provider = secret_provider::create(source);
    if (!provider) {
        {
            std::lock_guard lk(state.mtx);
            state.credentials_error = "no provider for '" + source + "'";
        }
        state.credentials_failed = true;
        wrk.notify_ui();
        co_return;
    }

    std::string api_key_name, ed25519_key_name, secret_key_name;
    if (source == "env") {
        api_key_name = "BINANCE_API_KEY";
        ed25519_key_name = "BINANCE_ED25519_PRIVATE_KEY";
        secret_key_name = "BINANCE_SECRET_KEY";
    } else if (source.starts_with("libsecret:")) {
        auto profile = source.substr(10);
        api_key_name = profile + "/api_key";
        ed25519_key_name = profile + "/ed25519_private_key";
        secret_key_name = profile + "/secret_key";
    } else {
        api_key_name = "api_key";
        ed25519_key_name = "ed25519_private_key";
        secret_key_name = "secret_key";
    }

    auto r = co_await binapi2::fapi::async_load_credentials(
        api.configuration(), *provider, api_key_name, ed25519_key_name, secret_key_name);

    if (!r) {
        {
            std::lock_guard lk(state.mtx);
            state.credentials_error = r.err.message;
        }
        state.credentials_failed = true;
    } else {
        state.credentials_loaded = true;
    }
    wrk.notify_ui();
}

} // namespace

worker::worker(app_state& state, ftxui::ScreenInteractive& screen, binapi2::fapi::config cfg)
    : state_(state)
    , screen_(screen)
    , work_guard_(boost::asio::make_work_guard(ioc_))
    , api_(std::make_unique<binapi2::futures_usdm_api>(std::move(cfg)))
{
}

worker::~worker()
{
    stop();
}

void worker::start()
{
    if (started_) return;
    started_ = true;
    thread_ = std::thread([this] {
        try {
            ioc_.run();
        } catch (const std::exception& e) {
            // Cannot use spdlog here — TUI owns the terminal. Last-resort
            // diagnostic into app_state for the status bar to render.
            std::lock_guard lk(state_.mtx);
            state_.status_message = std::string("worker error: ") + e.what();
            screen_.PostEvent(ftxui::Event::Custom);
        }
    });
    load_credentials_async();
}

void worker::stop()
{
    if (!started_) return;
    work_guard_.reset();
    ioc_.stop();
    if (thread_.joinable()) thread_.join();
    started_ = false;
}

void worker::notify_ui()
{
    // PostEvent is thread-safe; wakes Screen::Loop() to re-render.
    screen_.PostEvent(ftxui::Event::Custom);
}

void worker::load_credentials_async()
{
    // Spawn the cobalt task on this io_context's executor and detach it.
    // boost::cobalt::spawn returns a future when used with use_future; we
    // simply discard it (fire-and-forget) — the task updates state_ and
    // notifies the UI on completion.
    auto fut = boost::cobalt::spawn(
        ioc_.get_executor(),
        load_credentials_task(state_, *api_, *this),
        boost::asio::use_future);
    (void)fut;  // detach
}

} // namespace demo_ui
