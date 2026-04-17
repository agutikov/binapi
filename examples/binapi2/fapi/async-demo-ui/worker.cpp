// SPDX-License-Identifier: Apache-2.0

#include "worker.hpp"

#include <binapi2/demo_commands/result_sink.hpp>
#include <binapi2/fapi/secret_config.hpp>
#include <binapi2/fapi/transport_logger.hpp>
#include <secret_provider/factory.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_thread.hpp>

#include <ftxui/component/event.hpp>

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <future>

namespace demo_ui {

namespace {

/// Cobalt task that loads API credentials and updates `state` accordingly.
///
/// Logs go through spdlog: when the user started the UI with `--log-file`
/// these land in the file, otherwise they're dropped by the null sink.
/// Either way they never touch the terminal.
boost::cobalt::task<void>
load_credentials_task(app_state& state, binapi2::futures_usdm_api& api,
                      worker& wrk)
{
    // Resolve the same default the CLI uses: libsecret:demo unless overridden
    // via env var BINAPI_DEMO_SECRETS (read once at startup).
    const char* override_src = std::getenv("BINAPI_DEMO_SECRETS");
    std::string source = override_src && *override_src ? override_src : "libsecret:demo";

    spdlog::info("credentials: loading from '{}'", source);

    auto provider = secret_provider::create(source);
    if (!provider) {
        const auto msg = "no provider for '" + source + "'";
        spdlog::error("credentials: {}", msg);
        {
            std::lock_guard lk(state.mtx);
            state.credentials_error = msg;
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
        spdlog::error("credentials: load failed: {}", r.err.message);
        {
            std::lock_guard lk(state.mtx);
            state.credentials_error = r.err.message;
        }
        state.credentials_failed = true;
    } else {
        spdlog::info("credentials: loaded ok");
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
    install_transport_logger();
}

void worker::install_transport_logger()
{
    // The fapi config's logger is invoked from inside the transport
    // layer for every HTTP message sent / received. We dispatch each
    // entry to whichever request_capture is currently "active", set
    // via `active_capture_guard` inside the cobalt task body.
    //
    // The single-threaded worker io_context guarantees that at most one
    // REST call is dispatching at a time, so a single atomic pointer is
    // sufficient — we don't need an id map yet.
    api_->configuration().logger =
        [this](const binapi2::fapi::transport_log_entry& e) {
            auto* cap = active_capture_.load(std::memory_order_acquire);
            spdlog::debug("cfg.logger: dir={} proto={} method={} target={} raw={}b body={}b cap={}",
                          static_cast<int>(e.direction), e.protocol, e.method,
                          e.target, e.raw.size(), e.body.size(),
                          static_cast<void*>(cap));
            if (!cap) return;

            // The transport layer emits two flavours of events:
            //
            //   * `CONN` lifecycle steps (DNS resolve, TCP connect,
            //     TLS handshake) with empty `raw` — these are noise
            //     for the Raw view; we fold them into the info_lines
            //     channel so the user can still see them in the
            //     params pane if they care.
            //   * `HTTP` (and later `WS`) entries with a full `raw`
            //     payload — the actual wire bytes go into request.raw
            //     / response.raw.
            std::lock_guard lk(cap->mtx);
            if (e.protocol == "CONN") {
                std::string line = e.method + " " + e.target;
                if (!e.body.empty()) line += " (" + e.body + ")";
                if (!cap->info_lines.empty()) cap->info_lines += '\n';
                cap->info_lines += "[" + e.protocol + "] " + line;
            } else {
                // HTTP / WS — `raw` is the full serialized message
                // (request line + headers + body, or status line +
                // headers + body). Falls back to a synthesised line
                // for WS messages where `raw` is empty.
                const std::string text = !e.raw.empty()
                    ? e.raw
                    : (e.method + " " + e.target +
                       (e.body.empty() ? "" : "\n\n" + e.body));

                auto& side = (e.direction == binapi2::fapi::transport_direction::sent)
                    ? cap->request : cap->response;
                if (!side.raw.empty()) side.raw += "\n";
                side.raw += text;
            }
            screen_.PostEvent(ftxui::Event::Custom);
        };
}

worker::~worker()
{
    spdlog::debug("worker::~worker entered");
    stop();
    spdlog::debug("worker::~worker returning");
}

void worker::start()
{
    if (started_) return;
    started_ = true;
    spdlog::info("worker: starting io_context thread");
    thread_ = std::thread([this] {
        // Set the cobalt thread-local executor for this thread.
        // Without this, cobalt generators (which start eagerly with
        // initial_suspend = suspend_never) fall back to
        // `this_thread::get_executor()` to find their executor. If
        // it's not set, async operations inside the generator body
        // (like WebSocket connect in market_stream::subscribe) post
        // to a null executor and hang forever.
        boost::cobalt::this_thread::set_executor(ioc_.get_executor());
        try {
            ioc_.run();
        } catch (const std::exception& e) {
            // spdlog is routed to either the file (via --log-file) or a
            // null sink — never the terminal — so this is safe even under
            // an active TUI. Also surface the message in the status bar.
            spdlog::error("worker: io_context terminated: {}", e.what());
            {
                std::lock_guard lk(state_.mtx);
                state_.status_message = std::string("worker error: ") + e.what();
            }
            screen_.PostEvent(ftxui::Event::Custom);
        }
        spdlog::info("worker: io_context thread exiting");
    });
    load_credentials_async();
}

void worker::stop()
{
    if (!started_) return;
    spdlog::info("worker: stopping");
    work_guard_.reset();
    ioc_.stop();
    if (thread_.joinable()) thread_.join();
    started_ = false;
    spdlog::info("worker: stopped");
}

void worker::notify_ui()
{
    // PostEvent is thread-safe; wakes Screen::Loop() to re-render.
    screen_.PostEvent(ftxui::Event::Custom);
}

boost::cobalt::task<binapi2::fapi::rest::client*>
worker::acquire_rest_client(binapi2::demo::result_sink& sink)
{
    if (rest_client_) {
        spdlog::debug("worker::acquire_rest_client: returning cached client");
        co_return rest_client_.get();
    }

    spdlog::info("worker: connecting REST client (first use)");
    auto r = co_await api_->create_rest_client();
    if (!r) {
        spdlog::error("worker: REST connect failed: {}", r.err.message);
        sink.on_error(r.err);
        sink.on_done(1);
        co_return nullptr;
    }
    rest_client_ = std::move(*r);
    spdlog::info("worker: REST client connected, will be reused for subsequent calls");
    co_return rest_client_.get();
}

boost::cobalt::task<binapi2::fapi::rest::client*>
worker::acquire_rest_client_raw()
{
    if (rest_client_) {
        spdlog::debug("worker::acquire_rest_client_raw: returning cached client");
        co_return rest_client_.get();
    }

    spdlog::info("worker: connecting REST client (first use, raw path)");
    auto r = co_await api_->create_rest_client();
    if (!r) {
        spdlog::error("worker: REST connect failed: {}", r.err.message);
        co_return nullptr;
    }
    rest_client_ = std::move(*r);
    spdlog::info("worker: REST client connected");
    co_return rest_client_.get();
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
