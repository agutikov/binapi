// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-demo-cli — demonstration client and usage example for the
// binapi2 fapi library.  Each source file in this directory shows a different
// aspect of the library (REST, WebSocket API, streams, local order book).
//
// Usage:
//   binapi2-fapi-demo-cli [flags] <command> [args...]
//
// Flags:
//   -v                      Print full JSON responses
//   -vv                     Print JSON + transport log (method, target, status, body)
//   -vvv                    Print JSON + full HTTP with headers
//   -l, --live                  Use production endpoints (default: testnet)
//       --testnet               Use testnet endpoints (default)
//   -S, --save-request <file>   Save request to file
//   -R, --save-response <file>  Save response body to file
//   -r, --record <file>         Record raw WebSocket stream frames to JSONL file
//   -L, --log-file <file>       Log to file
//   -F, --file-loglevel <lvl>   File log level (trace/debug/info/warn/error/off)
//   -O, --stdout-loglevel <lvl> Stdout log level (trace/debug/info/warn/error/off)
//   -h, --help                  Print this help message

#include "common.hpp"
#include "cmd_market_data.hpp"
#include "cmd_account.hpp"
#include "cmd_trade.hpp"
#include "cmd_ws_api.hpp"
#include "cmd_stream.hpp"
#include "cmd_user_stream.hpp"
#include "cmd_order_book.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <string_view>

namespace {

struct command_entry
{
    std::string_view name;
    int (*fn)(const demo::args_t& args);
    std::string_view help;
};

// clang-format off
constexpr command_entry commands[] = {
    // Market data (public, sync)
    { "ping",                    demo::cmd_ping,                    "Test API connectivity" },
    { "time",                    demo::cmd_time,                    "Get server time" },
    { "exchange-info",           demo::cmd_exchange_info,           "Exchange info [symbol]" },
    { "order-book",              demo::cmd_order_book,              "Order book <symbol> [limit]" },
    { "recent-trades",           demo::cmd_recent_trades,           "Recent trades <symbol> [limit]" },
    { "book-ticker",             demo::cmd_book_ticker,             "Book ticker <symbol>" },
    { "book-tickers",            demo::cmd_book_tickers,            "All book tickers" },
    { "price-ticker",            demo::cmd_price_ticker,            "Price ticker <symbol>" },
    { "price-tickers",           demo::cmd_price_tickers,           "All price tickers" },
    { "ticker-24hr",             demo::cmd_ticker_24hr,             "24hr ticker <symbol>" },
    { "mark-price",              demo::cmd_mark_price,              "Mark price <symbol>" },
    { "mark-prices",             demo::cmd_mark_prices,             "All mark prices" },
    { "klines",                  demo::cmd_klines,                  "Klines <symbol> <interval> [limit]" },
    { "funding-rate",            demo::cmd_funding_rate,            "Funding rate history <symbol> [limit]" },
    { "open-interest",           demo::cmd_open_interest,           "Open interest <symbol>" },

    // Account (auth, sync)
    { "account-info",            demo::cmd_account_info,            "Account information (auth)" },
    { "balances",                demo::cmd_balances,                "Account balances (auth)" },
    { "position-risk",           demo::cmd_position_risk,           "Position risk [symbol] (auth)" },
    { "income-history",          demo::cmd_income_history,          "Income history [symbol] [limit] (auth)" },

    // Trade (auth, sync)
    { "new-order",               demo::cmd_new_order,               "Place order <sym> <side> <type> [-q Q] [-p P] [-t TIF]" },
    { "test-order",              demo::cmd_test_order,              "Test order (validates, does not place)" },
    { "cancel-order",            demo::cmd_cancel_order,            "Cancel order <symbol> <orderId>" },
    { "query-order",             demo::cmd_query_order,             "Query order <symbol> <orderId>" },
    { "open-orders",             demo::cmd_open_orders,             "Open orders [symbol]" },

    // WebSocket API (auth, sync)
    { "ws-logon",                demo::cmd_ws_logon,                "WebSocket API session logon (auth)" },
    { "ws-account-status",       demo::cmd_ws_account_status,       "Account status via WS API (auth)" },
    { "ws-order-place",          demo::cmd_ws_order_place,          "Place order via WS API <sym> <side> <type> [-q Q] [-p P]" },
    { "ws-order-cancel",         demo::cmd_ws_order_cancel,         "Cancel order via WS API <symbol> <orderId>" },

    // Market data streams
    { "stream-book-ticker",      demo::cmd_stream_book_ticker,      "Book ticker stream <symbol>" },
    { "stream-mark-price",       demo::cmd_stream_mark_price,       "Mark price stream <symbol>" },
    { "stream-kline",            demo::cmd_stream_kline,            "Kline stream <symbol> <interval>" },
    { "stream-ticker",           demo::cmd_stream_ticker,           "24hr ticker stream <symbol>" },
    { "stream-depth",            demo::cmd_stream_depth,            "Partial depth stream <symbol> [levels]" },
    { "stream-all-book-tickers", demo::cmd_stream_all_book_tickers, "All book tickers stream" },
    { "stream-all-tickers",      demo::cmd_stream_all_tickers,      "All 24hr tickers stream" },
    { "stream-all-mini-tickers", demo::cmd_stream_all_mini_tickers, "All mini tickers stream" },
    { "stream-liquidation",      demo::cmd_stream_liquidation,      "Liquidation orders stream <symbol>" },

    // User data
    { "listen-key-start",        demo::cmd_listen_key_start,        "Start listen key (auth)" },
    { "listen-key-keepalive",    demo::cmd_listen_key_keepalive,    "Keepalive listen key (auth)" },
    { "listen-key-close",        demo::cmd_listen_key_close,        "Close listen key (auth)" },
    { "user-stream",             demo::cmd_user_stream,             "User data stream demo (auth)" },

    // Local order book
    { "order-book-live",         demo::cmd_order_book_live,         "Live order book <symbol> [depth]" },
};
// clang-format on

void print_usage(const char* prog)
{
    std::cout << "Usage: " << prog << " [flags] <command> [args...]\n\n"
              << "Flags:\n"
              << "  -v                          Print full JSON responses\n"
              << "  -vv                         Print JSON + transport log\n"
              << "  -vvv                        Print JSON + full HTTP with headers\n"
              << "  -l, --live                  Use production endpoints (default: testnet)\n"
              << "      --testnet               Use testnet endpoints (default)\n"
              << "  -S, --save-request <file>   Save request to file\n"
              << "  -R, --save-response <file>  Save response body to file\n"
              << "  -r, --record <file>         Record raw stream frames to JSONL file\n"
              << "  -L, --log-file <file>       Log to file\n"
              << "  -F, --file-loglevel <lvl>   File log level (trace/debug/info/warn/error/off)\n"
              << "  -O, --stdout-loglevel <lvl> Stdout log level (trace/debug/info/warn/error/off)\n"
              << "  -h, --help                  Print this help\n\n"
              << "Commands:\n";
    for (const auto& cmd : commands)
        std::cout << "  " << cmd.name << std::string(25 - std::min(cmd.name.size(), std::size_t{24}), ' ')
                  << cmd.help << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2) { print_usage(argv[0]); return 1; }

    // Strip global flags, collect remaining args.
    demo::args_t cmd_args;
    const char* prog = argv[0];

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "-vvv")      { demo::verbosity = 3; }
        else if (arg == "-vv")  { demo::verbosity = 2; }
        else if (arg == "-v")   { demo::verbosity = 1; }
        else if (arg == "--live" || arg == "--prod" || arg == "-l") { demo::use_testnet = false; }
        else if (arg == "--testnet") { demo::use_testnet = true; }
        else if ((arg == "--save-request"  || arg == "-S") && i + 1 < argc) { demo::save_request_file  = argv[++i]; }
        else if ((arg == "--save-response" || arg == "-R") && i + 1 < argc) { demo::save_response_file = argv[++i]; }
        else if ((arg == "--record"        || arg == "-r") && i + 1 < argc) { demo::record_file        = argv[++i]; }
        else if ((arg == "--log-file"      || arg == "-L") && i + 1 < argc) { demo::log_file           = argv[++i]; }
        else if ((arg == "--file-loglevel" || arg == "-F") && i + 1 < argc) { demo::file_loglevel      = argv[++i]; }
        else if ((arg == "--stdout-loglevel" || arg == "-O") && i + 1 < argc) { demo::stdout_loglevel  = argv[++i]; }
        else if (arg == "--help" || arg == "-h") { print_usage(prog); return 0; }
        else { cmd_args.emplace_back(arg); }
    }

    if (cmd_args.empty()) { print_usage(prog); return 1; }

    demo::init_logging();

    std::string_view cmd_name = cmd_args[0];
    spdlog::info("command: {}", cmd_name);

    // Remove the command name, pass only subcommand args.
    demo::args_t sub_args(cmd_args.begin() + 1, cmd_args.end());

    for (const auto& cmd : commands) {
        if (cmd.name == cmd_name) {
            return cmd.fn(sub_args);
        }
    }

    std::cerr << "unknown command: " << cmd_name << "\n\n";
    print_usage(prog);
    return 1;
}
