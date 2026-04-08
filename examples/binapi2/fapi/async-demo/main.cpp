// SPDX-License-Identifier: Apache-2.0
//
// Async demo: cobalt::main entry point with REST, WebSocket stream, and WS API.
//
// Demonstrates pure coroutine-based usage without sync wrappers.
// All three protocols (REST, WS stream, WS API) run on cobalt's io_context
// via co_await — no io_thread needed for async calls.
//
// Usage:
//   async-demo [rest|stream|ws-api|all]
//
// Env: BINANCE_API_KEY, BINANCE_SECRET_KEY (for ws-api)

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>
#include <binapi2/fapi/transport/ssl_context.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/streams.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>

#include <glaze/glaze.hpp>

#include <boost/cobalt/main.hpp>
#include <boost/cobalt/op.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

using namespace binapi2::fapi;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static config make_config()
{
    auto cfg = config::testnet_config();
    if (const char* k = std::getenv("BINANCE_API_KEY"))    cfg.api_key = k;
    if (const char* s = std::getenv("BINANCE_SECRET_KEY")) cfg.secret_key = s;
    return cfg;
}

template<typename T>
static void print_json(const T& value)
{
    if (auto j = glz::write<glz::opts{.prettify = true}>(value))
        std::cout << *j << '\n';
}

// ---------------------------------------------------------------------------
// 1. REST — truly async via client::async_execute
//
// The coroutine chain: async_execute -> async_prepare_and_send -> http_client::async_request
// All co_await points use co_main's executor. io_thread exists inside client
// but is unused for async calls — the coroutine runs entirely on our executor.
// ---------------------------------------------------------------------------

static boost::cobalt::task<int> demo_rest(config cfg)
{
    std::cout << "=== REST (async) ===\n";

    // client creates io_thread internally, but async_execute bypasses it.
    client c(std::move(cfg));

    // co_await drives the HTTP coroutine on cobalt::main's io_context.
    auto r = co_await c.market_data.async_execute(types::server_time_request_t{});
    if (!r) {
        std::cerr << "FAIL server_time: " << r.err.message << '\n';
        co_return 1;
    }
    std::cout << "server_time: " << r->serverTime.value << '\n';

    auto info = co_await c.market_data.async_execute(
        types::exchange_info_request_t{});
    if (!info) {
        std::cerr << "FAIL exchange_info: " << info.err.message << '\n';
        co_return 1;
    }
    std::cout << "exchange_info: " << info->symbols.size() << " symbols\n";

    auto book = co_await c.market_data.async_execute(
        types::order_book_request_t{.symbol = "BTCUSDT", .limit = 5});
    if (!book) {
        std::cerr << "FAIL order_book: " << book.err.message << '\n';
        co_return 1;
    }
    std::cout << "order_book: " << book->bids.size() << " bids, "
              << book->asks.size() << " asks\n";
    if (!book->asks.empty()) {
        std::cout << "  best ask: " << book->asks[0].price << " x "
                  << book->asks[0].quantity << '\n';
    }

    std::cout << "REST OK\n\n";
    co_return 0;
}

// ---------------------------------------------------------------------------
// 2. WebSocket stream — async connect + read loop via transport layer
//
// market_streams doesn't expose cobalt::task-based async read, so we use
// transport::websocket_client directly. This is what a proper async stream
// consumer looks like: co_await each read, parse, process.
// ---------------------------------------------------------------------------

static boost::cobalt::task<int> demo_stream(config cfg)
{
    std::cout << "=== Stream (async) ===\n";

    // We need an io_thread for websocket_client's constructor signature,
    // but async methods bypass it entirely — they use our coroutine's executor.
    detail::io_thread io;
    transport::websocket_client ws(io, cfg);

    const std::string target = cfg.stream_base_target + "/btcusdt@bookTicker";
    std::cout << "connecting to " << cfg.stream_host << target << " ...\n";

    auto conn = co_await ws.async_connect(cfg.stream_host, cfg.stream_port, target);
    if (!conn) {
        std::cerr << "FAIL connect: " << conn.err.message << '\n';
        co_return 1;
    }
    std::cout << "connected, reading up to 10 frames...\n";

    int count = 0;
    while (count < 10) {
        auto msg = co_await ws.async_read_text();
        if (!msg) {
            // Stream killed (timeout) or server closed — not necessarily an error.
            std::cout << "(stream ended: " << msg.err.message << ")\n";
            break;
        }

        types::book_ticker_stream_event_t event{};
        glz::context ctx{};
        if (glz::read<detail::json_read_opts>(event, *msg, ctx)) {
            std::cerr << "parse error, raw: " << msg->substr(0, 200) << '\n';
            continue;
        }

        std::cout << "  [" << ++count << "] " << event.symbol
                  << "  bid=" << event.best_bid_price
                  << "  ask=" << event.best_ask_price << '\n';
    }

    auto cl = co_await ws.async_close();
    (void)cl;

    std::cout << "Stream OK (" << count << " frames)\n\n";
    co_return 0;
}

// ---------------------------------------------------------------------------
// 3. WebSocket API — async connect + raw JSON-RPC via transport layer
//
// websocket_api::client's async_execute wraps sync internally (send_rpc uses
// transport_.write_text which calls run_sync). To get true async, we bypass
// the high-level client and speak JSON-RPC directly over async transport.
// This shows the raw protocol: send JSON text frame, co_await response frame.
// ---------------------------------------------------------------------------

static boost::cobalt::task<int> demo_ws_api(config cfg)
{
    std::cout << "=== WS API (async) ===\n";

    if (cfg.api_key.empty()) {
        std::cerr << "BINANCE_API_KEY not set, skipping WS API demo.\n\n";
        co_return 0;
    }

    detail::io_thread io;
    transport::websocket_client ws(io, cfg);

    std::cout << "connecting to " << cfg.websocket_api_host
              << cfg.websocket_api_target << " ...\n";

    auto conn = co_await ws.async_connect(
        cfg.websocket_api_host, cfg.websocket_api_port, cfg.websocket_api_target);
    if (!conn) {
        std::cerr << "FAIL connect: " << conn.err.message << '\n';
        co_return 1;
    }
    std::cout << "connected\n";

    // Helper: send JSON-RPC request, co_await response.
    auto rpc = [&](const std::string& /*id*/, const std::string& payload)
        -> boost::cobalt::task<result<std::string>>
    {
        auto wr = co_await ws.async_write_text(payload);
        if (!wr) co_return result<std::string>::failure(wr.err);

        auto rd = co_await ws.async_read_text();
        co_return rd;
    };

    // --- Session logon ---
    {
        query_map auth;
        inject_auth_query(auth, cfg.recv_window, current_timestamp_ms());
        sign_query(auth, cfg.secret_key);

        // Build session.logon JSON-RPC envelope as raw JSON string.
        std::string payload =
            R"({"id":"1","method":"session.logon","params":{)"
            R"("apiKey":")" + cfg.api_key + R"(",)"
            R"("timestamp":")" + auth["timestamp"] + R"(",)"
            R"("signature":")" + auth["signature"] + R"("}})";

        std::cout << "session.logon ...\n";
        auto resp = co_await rpc("1", payload);
        if (!resp) {
            std::cerr << "FAIL logon: " << resp.err.message << '\n';
            co_return 1;
        }
        std::cout << "  logon response: " << resp->substr(0, 200) << '\n';
    }

    // --- Book ticker (unsigned, session auth covers it) ---
    {
        std::string payload =
            R"({"id":"2","method":"v2/ticker.book","params":{"symbol":"BTCUSDT"}})";


        std::cout << "v2/ticker.book BTCUSDT ...\n";
        auto resp = co_await rpc("2", payload);
        if (!resp) {
            std::cerr << "FAIL ticker.book: " << resp.err.message << '\n';
            co_return 1;
        }

        // Parse the response to show structured data.
        types::websocket_api_response_t<types::book_ticker_t> parsed{};
        glz::context ctx{};
        if (!glz::read<detail::json_read_opts>(parsed, *resp, ctx) && parsed.result) {
            std::cout << "  " << parsed.result->symbol
                      << "  bid=" << parsed.result->bidPrice
                      << "  ask=" << parsed.result->askPrice << '\n';
        } else {
            std::cout << "  raw: " << resp->substr(0, 200) << '\n';
        }
    }

    auto cl = co_await ws.async_close();
    (void)cl;

    std::cout << "WS API OK\n\n";
    co_return 0;
}

// ---------------------------------------------------------------------------
// Entry point: cobalt::main provides its own io_context and event loop.
// No manual io_context::run() needed.
// ---------------------------------------------------------------------------

boost::cobalt::main co_main(int argc, char* argv[])
{
    std::string_view mode = argc > 1 ? argv[1] : "all";
    auto cfg = make_config();

    int rc = 0;

    if (mode == "rest" || mode == "all") {
        rc |= co_await demo_rest(cfg);
    }
    if (mode == "stream" || mode == "all") {
        rc |= co_await demo_stream(cfg);
    }
    if (mode == "ws-api" || mode == "all") {
        rc |= co_await demo_ws_api(cfg);
    }

    co_return rc;
}
