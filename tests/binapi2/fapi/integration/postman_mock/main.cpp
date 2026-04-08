// SPDX-License-Identifier: Apache-2.0
//
// Integration test: exercises the binapi2 client against the Postman mock server
// (nginx with static JSON responses and TLS).
//
// Uses cobalt::main for async execution — all API calls are co_awaited.
//
// Prerequisites:
//   1. scripts/api/postman_mock/start.sh   (generates certs, starts docker compose)
//   2. Set SSL_CERT_FILE to the mock server's CA cert so OpenSSL trusts it.
//
// The test binary accepts two optional env vars:
//   MOCK_HOST  — default "localhost"
//   MOCK_PORT  — default "8443"

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <boost/cobalt/main.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

using namespace binapi2::fapi;

// ---------------------------------------------------------------------------
// Helpers.
// ---------------------------------------------------------------------------

std::string
env_or(const char* name, const char* fallback)
{
    const char* v = std::getenv(name);
    return v ? v : fallback;
}

int failures = 0;
int passed = 0;

template<typename T>
bool
check(const char* name, const result<T>& r)
{
    if (!r) {
        std::cerr << "FAIL " << name << ": [" << static_cast<int>(r.err.code) << "] "
                  << r.err.message << "\n";
        if (!r.err.payload.empty())
            std::cerr << "     payload: " << r.err.payload << "\n";
        ++failures;
        return false;
    }
    std::cout << "  OK " << name << "\n";
    ++passed;
    return true;
}

} // anonymous namespace

// ===========================================================================

boost::cobalt::main
co_main(int, char*[])
{
    const auto host = env_or("MOCK_HOST", "localhost");
    const auto port = env_or("MOCK_PORT", "8443");

    std::cout << "binapi2 integration test against mock server " << host << ":" << port << "\n\n";

    config cfg;
    cfg.rest_host = host;
    cfg.rest_port = port;
    cfg.api_key = "test-api-key";
    cfg.secret_key = "test-secret-key";
    cfg.ca_cert_file = env_or("SSL_CERT_FILE", "");

    client c(cfg, async_mode);

    // =======================================================================
    // 1. Public market data endpoints (no auth).
    // =======================================================================

    std::cout << "--- Market data ---\n";

    // Ping
    {
        auto r = co_await c.market_data.async_execute(types::ping_request_t{});
        check("ping", r);
    }

    // Server time
    {
        auto r = co_await c.market_data.async_execute(types::server_time_request_t{});
        if (check("server_time", r)) {
            if (r->serverTime.value == 0) {
                std::cerr << "FAIL server_time: serverTime is 0\n";
                ++failures;
                --passed;
            }
        }
    }

    // Exchange info
    {
        auto r = co_await c.market_data.async_execute(types::exchange_info_request_t{});
        if (check("exchange_info", r)) {
            if (r->symbols.empty()) {
                std::cerr << "FAIL exchange_info: symbols array is empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Order book
    {
        auto r = co_await c.market_data.async_execute(
            types::order_book_request_t{.symbol = "BTCUSDT", .limit = 5});
        if (check("order_book", r)) {
            if (r->bids.empty() || r->asks.empty()) {
                std::cerr << "FAIL order_book: bids or asks empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Recent trades
    {
        auto r = co_await c.market_data.async_execute(
            types::recent_trades_request_t{.symbol = "BTCUSDT"});
        if (check("recent_trades", r)) {
            if (r->empty()) {
                std::cerr << "FAIL recent_trades: empty array\n";
                ++failures;
                --passed;
            }
        }
    }

    // Klines
    {
        auto r = co_await c.market_data.async_klines(
            types::kline_request_t{.symbol = "BTCUSDT", .interval = types::kline_interval_t::h1});
        if (check("klines", r)) {
            if (r->empty()) {
                std::cerr << "FAIL klines: empty array\n";
                ++failures;
                --passed;
            }
        }
    }

    // Price ticker
    {
        auto r = co_await c.market_data.async_execute(
            types::price_ticker_request_t{.symbol = "BTCUSDT"});
        if (check("price_ticker", r)) {
            if (r->symbol.empty()) {
                std::cerr << "FAIL price_ticker_t: symbol empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Book ticker
    {
        auto r = co_await c.market_data.async_execute(
            types::book_ticker_request_t{.symbol = "BTCUSDT"});
        if (check("book_ticker", r)) {
            if (r->symbol.empty()) {
                std::cerr << "FAIL book_ticker_t: symbol empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Mark price
    {
        auto r = co_await c.market_data.async_execute(
            types::mark_price_request_t{.symbol = "BTCUSDT"});
        if (check("mark_price", r)) {
            if (r->symbol.empty()) {
                std::cerr << "FAIL mark_price_t: symbol empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Funding rate history
    {
        auto r = co_await c.market_data.async_execute(
            types::funding_rate_history_request_t{.symbol = "BTCUSDT"});
        if (check("funding_rate_history", r)) {
            if (r->empty()) {
                std::cerr << "FAIL funding_rate_history: empty array\n";
                ++failures;
                --passed;
            }
        }
    }

    // =======================================================================
    // 2. Account endpoints (signed).
    // =======================================================================

    std::cout << "\n--- Account ---\n";

    // Account information
    {
        auto r = co_await c.account.async_account_information();
        if (check("account_information", r)) {
            if (r->assets.empty()) {
                std::cerr << "FAIL account_information_t: assets empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Balances
    {
        auto r = co_await c.account.async_balances();
        if (check("balances", r)) {
            if (r->empty()) {
                std::cerr << "FAIL balances: empty array\n";
                ++failures;
                --passed;
            }
        }
    }

    // Position risk
    {
        auto r = co_await c.account.async_execute(types::position_risk_request_t{});
        if (check("position_risk", r)) {
            if (r->empty()) {
                std::cerr << "FAIL position_risk_t: empty array\n";
                ++failures;
                --passed;
            }
        }
    }

    // =======================================================================
    // 3. Trade endpoints (signed).
    // =======================================================================

    std::cout << "\n--- Trade ---\n";

    // Query order
    {
        auto r = co_await c.trade.async_execute(
            types::query_order_request_t{.symbol = "BTCUSDT", .orderId = 22542179});
        if (check("query_order", r)) {
            if (r->symbol != "BTCUSDT") {
                std::cerr << "FAIL query_order: unexpected symbol: " << r->symbol << "\n";
                ++failures;
                --passed;
            }
        }
    }

    // All open orders
    {
        auto r = co_await c.trade.async_execute(types::all_open_orders_request_t{});
        if (check("all_open_orders", r)) {
            if (r->empty()) {
                std::cerr << "FAIL all_open_orders: empty array\n";
                ++failures;
                --passed;
            }
        }
    }

    // =======================================================================
    // 4. User data streams.
    // =======================================================================

    std::cout << "\n--- User data streams ---\n";

    // Start listen key
    {
        auto r = co_await c.user_data_streams.async_start();
        if (check("start_listen_key", r)) {
            if (r->listenKey.empty()) {
                std::cerr << "FAIL start_listen_key: listenKey empty\n";
                ++failures;
                --passed;
            }
        }
    }

    // Keepalive listen key
    {
        auto r = co_await c.user_data_streams.async_keepalive();
        check("keepalive_listen_key", r);
    }

    // Close listen key
    {
        auto r = co_await c.user_data_streams.async_close();
        check("close_listen_key", r);
    }

    // =======================================================================
    // Summary.
    // =======================================================================

    std::cout << "\n========================================\n";
    std::cout << "Passed: " << passed << "  Failed: " << failures << "\n";

    co_return failures > 0 ? 1 : 0;
}
