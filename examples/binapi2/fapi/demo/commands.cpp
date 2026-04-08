// SPDX-License-Identifier: Apache-2.0
//
// Shared demo command implementations — all coroutines using co_await.

#include "commands.hpp"

#include <binapi2/fapi/detail/json_opts.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace demo {

using namespace binapi2::fapi;

// ---------------------------------------------------------------------------
// REST commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int>
cmd_ping(client& c, const args_t& /*args*/)
{
    auto r = co_await c.market_data.async_execute(types::ping_request_t{});
    if (!r) { std::cerr << "FAIL: " << r.err.message << '\n'; co_return 1; }
    std::cout << "pong\n";
    co_return 0;
}

boost::cobalt::task<int>
cmd_server_time(client& c, const args_t& /*args*/)
{
    auto r = co_await c.market_data.async_execute(types::server_time_request_t{});
    if (!r) { std::cerr << "FAIL: " << r.err.message << '\n'; co_return 1; }
    std::cout << "server_time: " << r->serverTime.value << '\n';
    co_return 0;
}

boost::cobalt::task<int>
cmd_exchange_info(client& c, const args_t& /*args*/)
{
    auto r = co_await c.market_data.async_execute(types::exchange_info_request_t{});
    if (!r) { std::cerr << "FAIL: " << r.err.message << '\n'; co_return 1; }
    std::cout << r->symbols.size() << " symbols\n";
    print_json(*r);
    co_return 0;
}

boost::cobalt::task<int>
cmd_order_book(client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: order-book SYMBOL [limit]\n"; co_return 1; }
    types::order_book_request_t req{ .symbol = args[0] };
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    auto r = co_await c.market_data.async_execute(req);
    if (!r) { std::cerr << "FAIL: " << r.err.message << '\n'; co_return 1; }
    std::cout << r->bids.size() << " bids, " << r->asks.size() << " asks\n";
    print_json(*r);
    co_return 0;
}

// ---------------------------------------------------------------------------
// Stream command
// ---------------------------------------------------------------------------

boost::cobalt::task<int>
cmd_stream_book_ticker(client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-book-ticker SYMBOL\n"; co_return 1; }

    auto& streams = c.streams();
    const auto& cfg = streams.configuration();
    std::string symbol_lower = args[0];
    std::ranges::transform(symbol_lower, symbol_lower.begin(), ::tolower);
    const auto target = cfg.stream_base_target + "/" + symbol_lower + "@bookTicker";

    auto conn = co_await streams.async_connect(target);
    if (!conn) { std::cerr << "FAIL connect: " << conn.err.message << '\n'; co_return 1; }

    std::cout << "connected to " << args[0] << "@bookTicker, reading...\n";
    int count = 0;
    while (count < 20) {
        auto msg = co_await streams.async_read_text();
        if (!msg) { std::cout << "(stream ended: " << msg.err.message << ")\n"; break; }

        types::book_ticker_stream_event_t event{};
        glz::context ctx{};
        if (glz::read<detail::json_read_opts>(event, *msg, ctx)) {
            std::cerr << "parse error\n";
            continue;
        }
        std::cout << "  [" << ++count << "] " << event.symbol
                  << "  bid=" << event.best_bid_price
                  << "  ask=" << event.best_ask_price << '\n';
    }

    (void)co_await streams.async_close();
    co_return 0;
}

// ---------------------------------------------------------------------------
// WS API command
// ---------------------------------------------------------------------------

boost::cobalt::task<int>
cmd_ws_book_ticker(client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: ws-book-ticker SYMBOL\n"; co_return 1; }

    auto& ws = c.ws_api();
    auto conn = co_await ws.async_connect();
    if (!conn) { std::cerr << "FAIL connect: " << conn.err.message << '\n'; co_return 1; }

    if (!c.configuration().api_key.empty()) {
        auto logon = co_await ws.async_session_logon();
        if (!logon) { std::cerr << "FAIL logon: " << logon.err.message << '\n'; co_return 1; }
        std::cout << "session logon OK\n";
    }

    types::websocket_api_book_ticker_request_t req{ .symbol = args[0] };
    auto r = co_await ws.async_execute(req);
    if (!r) { std::cerr << "FAIL: " << r.err.message << '\n'; co_return 1; }

    if (r->result) {
        std::cout << r->result->symbol
                  << "  bid=" << r->result->bidPrice
                  << "  ask=" << r->result->askPrice << '\n';
    }
    print_json(*r);

    (void)co_await ws.async_close();
    co_return 0;
}

// ---------------------------------------------------------------------------
// Command table
// ---------------------------------------------------------------------------

static const command_entry commands[] = {
    { "ping",               cmd_ping,               "REST: ping the server" },
    { "server-time",        cmd_server_time,        "REST: get server time" },
    { "exchange-info",      cmd_exchange_info,      "REST: get exchange info" },
    { "order-book",         cmd_order_book,         "REST: order book SYMBOL [limit]" },
    { "stream-book-ticker", cmd_stream_book_ticker, "Stream: book ticker SYMBOL" },
    { "ws-book-ticker",     cmd_ws_book_ticker,     "WS API: book ticker SYMBOL" },
};

const command_entry&
find_command(std::string_view name)
{
    for (const auto& cmd : commands) {
        if (cmd.name == name) return cmd;
    }
    throw std::invalid_argument("unknown command: " + std::string(name));
}

void
print_help()
{
    std::cout << "Commands:\n";
    for (const auto& cmd : commands) {
        std::cout << "  " << cmd.name << " — " << cmd.help << '\n';
    }
}

} // namespace demo
