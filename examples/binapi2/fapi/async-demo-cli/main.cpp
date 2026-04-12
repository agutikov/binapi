// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-async-demo-cli — async demonstration client and usage example for
// the binapi2 fapi library.  Each source file in this directory shows a different
// aspect of the library (REST, WebSocket API, streams, local order book).
//
// Usage:
//   binapi2-fapi-async-demo-cli [flags] <command> [args...]
//
// Flags:
//   -v                      Print full JSON responses
//   -vv                     Print JSON + transport log (method, target, status, body)
//   -vvv                    Print JSON + full HTTP with headers
//   -l, --live                  Use production endpoints (default: testnet)
//       --testnet               Use testnet endpoints (default)
//   -S, --save-request <file>   Save request to file
//   -R, --save-response <file>  Save response body to file
//   -L, --log-file <file>       Log to file
//   -F, --file-loglevel <lvl>   File log level (trace/debug/info/warn/error/off)
//   -O, --stdout-loglevel <lvl> Stdout log level (trace/debug/info/warn/error/off)
//   -h, --help                  Print this help message

#include "common.hpp"
#include "cmd_market_data.hpp"
#include "cmd_account.hpp"
#include "cmd_trade.hpp"
#include "cmd_convert.hpp"
#include "cmd_ws_api.hpp"
#include "cmd_stream.hpp"
#include "cmd_user_stream.hpp"
#include "cmd_order_book.hpp"
#include "cmd_pipeline_order_book.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/streams/sinks/spdlog_sink.hpp>

#include <boost/asio/signal_set.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/this_coro.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string_view>

namespace {

struct command_entry
{
    std::string_view name;
    demo::command_fn fn;
    std::string_view help;
};

// Group header: fn == nullptr, name is the group title.
constexpr bool is_group(const command_entry& e) { return e.fn == nullptr; }

// clang-format off
constexpr command_entry commands[] = {
    { "Market Data", nullptr, {} },
    { "ping",                    demo::cmd_ping,                    "Test API connectivity" },
    { "time",                    demo::cmd_time,                    "Get server time" },
    { "exchange-info",           demo::cmd_exchange_info,           "Exchange info [symbol]" },
    { "order-book",              demo::cmd_order_book,              "Order book <symbol> [limit]" },
    { "recent-trades",           demo::cmd_recent_trades,           "Recent trades <symbol> [limit]" },
    { "aggregate-trades",        demo::cmd_aggregate_trades,        "Aggregate trades <symbol> [limit]" },
    { "historical-trades",       demo::cmd_historical_trades,       "Historical trades <symbol> [limit]" },
    { "book-ticker",             demo::cmd_book_ticker,             "Book ticker <symbol>" },
    { "book-tickers",            demo::cmd_book_tickers,            "All book tickers" },
    { "price-ticker",            demo::cmd_price_ticker,            "Price ticker <symbol>" },
    { "price-tickers",           demo::cmd_price_tickers,           "All price tickers" },
    { "price-ticker-v2",         demo::cmd_price_ticker_v2,         "Price ticker v2 <symbol>" },
    { "price-tickers-v2",        demo::cmd_price_tickers_v2,        "All price tickers v2" },
    { "ticker-24hr",             demo::cmd_ticker_24hr,             "24hr ticker <symbol>" },
    { "ticker-24hrs",            demo::cmd_ticker_24hrs,            "All 24hr tickers" },
    { "mark-price",              demo::cmd_mark_price,              "Mark price <symbol>" },
    { "mark-prices",             demo::cmd_mark_prices,             "All mark prices" },
    { "klines",                  demo::cmd_klines,                  "Klines <symbol> <interval> [limit]" },
    { "continuous-kline",        demo::cmd_continuous_kline,        "Continuous kline <pair> <interval> [limit]" },
    { "index-price-kline",       demo::cmd_index_price_kline,       "Index price kline <pair> <interval> [limit]" },
    { "mark-price-klines",       demo::cmd_mark_price_klines,       "Mark price klines <symbol> <interval> [limit]" },
    { "premium-index-klines",    demo::cmd_premium_index_klines,    "Premium index klines <symbol> <interval> [limit]" },
    { "funding-rate",            demo::cmd_funding_rate,            "Funding rate history <symbol> [limit]" },
    { "funding-rate-info",       demo::cmd_funding_rate_info,       "Funding rate info (all)" },
    { "open-interest",           demo::cmd_open_interest,           "Open interest <symbol>" },
    { "open-interest-stats",     demo::cmd_open_interest_stats,     "Open interest stats <symbol> <period> [limit]" },
    { "top-ls-account-ratio",    demo::cmd_top_ls_account_ratio,    "Top L/S account ratio <symbol> <period> [limit]" },
    { "top-ls-trader-ratio",     demo::cmd_top_ls_trader_ratio,     "Top L/S trader ratio <symbol> <period> [limit]" },
    { "long-short-ratio",        demo::cmd_long_short_ratio,        "Global L/S ratio <symbol> <period> [limit]" },
    { "taker-volume",            demo::cmd_taker_volume,            "Taker buy/sell volume <symbol> <period> [limit]" },
    { "basis",                   demo::cmd_basis,                   "Basis <pair> <period> [limit]" },
    { "delivery-price",          demo::cmd_delivery_price,          "Delivery price <pair>" },
    { "composite-index-info",    demo::cmd_composite_index_info,    "Composite index info [symbol]" },
    { "index-constituents",      demo::cmd_index_constituents,      "Index constituents <symbol>" },
    { "asset-index",             demo::cmd_asset_index,             "Asset index [symbol]" },
    { "insurance-fund",          demo::cmd_insurance_fund,          "Insurance fund [symbol]" },
    { "adl-risk",                demo::cmd_adl_risk,                "ADL risk [symbol]" },
    { "rpi-depth",               demo::cmd_rpi_depth,               "RPI depth <symbol> [limit]" },
    { "trading-schedule",        demo::cmd_trading_schedule,        "Trading schedule" },

    { "Account", nullptr, {} },
    { "account-info",            demo::cmd_account_info,            "Account information (auth)" },
    { "balances",                demo::cmd_balances,                "Account balances (auth)" },
    { "position-risk",           demo::cmd_position_risk,           "Position risk [symbol] (auth)" },
    { "income-history",          demo::cmd_income_history,          "Income history [symbol] [limit] (auth)" },
    { "account-config",          demo::cmd_account_config,          "Account config (auth)" },
    { "symbol-config",           demo::cmd_symbol_config,           "Symbol config [symbol] (auth)" },
    { "multi-assets-mode",       demo::cmd_multi_assets_mode,       "Get multi-assets mode (auth)" },
    { "position-mode",           demo::cmd_position_mode,           "Get position mode (auth)" },
    { "rate-limit-order",        demo::cmd_rate_limit_order,        "Rate limit order count (auth)" },
    { "leverage-bracket",        demo::cmd_leverage_bracket,        "Leverage brackets [symbol] (auth)" },
    { "commission-rate",         demo::cmd_commission_rate,         "Commission rate <symbol> (auth)" },
    { "bnb-burn",                demo::cmd_bnb_burn,                "Get BNB burn status (auth)" },
    { "toggle-bnb-burn",         demo::cmd_toggle_bnb_burn,         "Toggle BNB burn <true|false> (auth)" },
    { "quantitative-rules",      demo::cmd_quantitative_rules,      "Quantitative rules [symbol] (auth)" },
    { "pm-account-info",         demo::cmd_pm_account_info,         "Portfolio margin info <asset> (auth)" },
    { "download-id-transaction", demo::cmd_download_id_transaction, "Download ID transaction <start> <end> (auth)" },
    { "download-link-transaction", demo::cmd_download_link_transaction, "Download link transaction <id> (auth)" },
    { "download-id-order",       demo::cmd_download_id_order,       "Download ID order <start> <end> (auth)" },
    { "download-link-order",     demo::cmd_download_link_order,     "Download link order <id> (auth)" },
    { "download-id-trade",       demo::cmd_download_id_trade,       "Download ID trade <start> <end> (auth)" },
    { "download-link-trade",     demo::cmd_download_link_trade,     "Download link trade <id> (auth)" },

    { "Trade", nullptr, {} },
    { "new-order",               demo::cmd_new_order,               "Place order <sym> <side> <type> [-q Q] [-p P] [-t TIF]" },
    { "test-order",              demo::cmd_test_order,              "Test order (validates, does not place)" },
    { "modify-order",            demo::cmd_modify_order,            "Modify order <sym> <side> <orderId> -q Q -p P" },
    { "cancel-order",            demo::cmd_cancel_order,            "Cancel order <symbol> <orderId>" },
    { "cancel-multiple-orders",  demo::cmd_cancel_multiple_orders,  "Cancel orders <symbol> <id1,id2,...>" },
    { "cancel-all-orders",       demo::cmd_cancel_all_orders,       "Cancel all open orders <symbol>" },
    { "auto-cancel",             demo::cmd_auto_cancel,             "Auto-cancel <symbol> <countdownMs>" },
    { "query-order",             demo::cmd_query_order,             "Query order <symbol> <orderId>" },
    { "query-open-order",        demo::cmd_query_open_order,        "Query open order <symbol> <orderId>" },
    { "open-orders",             demo::cmd_open_orders,             "Open orders [symbol]" },
    { "all-orders",              demo::cmd_all_orders,              "All orders <symbol> [limit]" },
    { "position-info-v3",        demo::cmd_position_info_v3,        "Position info v3 [symbol]" },
    { "adl-quantile",            demo::cmd_adl_quantile,            "ADL quantile [symbol]" },
    { "force-orders",            demo::cmd_force_orders,            "Force orders [symbol] [limit]" },
    { "account-trades",          demo::cmd_account_trades,          "Account trades <symbol> [limit]" },
    { "change-position-mode",    demo::cmd_change_position_mode,    "Change position mode <true|false>" },
    { "change-multi-assets-mode", demo::cmd_change_multi_assets_mode, "Change multi-assets mode <true|false>" },
    { "change-leverage",         demo::cmd_change_leverage,         "Change leverage <symbol> <leverage>" },
    { "change-margin-type",      demo::cmd_change_margin_type,      "Change margin type <symbol> <ISOLATED|CROSSED>" },
    { "modify-isolated-margin",  demo::cmd_modify_isolated_margin,  "Modify isolated margin <sym> <amount> <1|2>" },
    { "position-margin-history", demo::cmd_position_margin_history, "Position margin history <symbol> [limit]" },
    { "order-modify-history",    demo::cmd_order_modify_history,    "Order modify history <symbol> [orderId]" },
    { "new-algo-order",          demo::cmd_new_algo_order,          "Place algo order <sym> <side> <type> <algo> -q Q [-p P]" },
    { "cancel-algo-order",       demo::cmd_cancel_algo_order,       "Cancel algo order <algoId>" },
    { "query-algo-order",        demo::cmd_query_algo_order,        "Query algo order <algoId>" },
    { "all-algo-orders",         demo::cmd_all_algo_orders,         "All algo orders <symbol> [limit]" },
    { "open-algo-orders",        demo::cmd_open_algo_orders,        "Open algo orders" },
    { "cancel-all-algo-orders",  demo::cmd_cancel_all_algo_orders,  "Cancel all algo orders" },
    { "tradfi-perps",            demo::cmd_tradfi_perps,            "TradFi perps" },

    { "Convert", nullptr, {} },
    { "convert-quote",           demo::cmd_convert_quote,           "Convert quote <from> <to> <amount> (auth)" },
    { "convert-accept",          demo::cmd_convert_accept,          "Accept convert quote <quoteId> (auth)" },
    { "convert-order-status",    demo::cmd_convert_order_status,    "Convert order status <orderId> (auth)" },

    { "WebSocket API", nullptr, {} },
    { "ws-logon",                demo::cmd_ws_logon,                "WebSocket API session logon (auth)" },
    { "ws-book-ticker",          demo::cmd_ws_book_ticker,          "Book ticker via WS API [symbol]" },
    { "ws-price-ticker",         demo::cmd_ws_price_ticker,         "Price ticker via WS API [symbol]" },
    { "ws-account-status",       demo::cmd_ws_account_status,       "Account status via WS API (auth)" },
    { "ws-account-status-v2",    demo::cmd_ws_account_status_v2,    "Account status v2 via WS API (auth)" },
    { "ws-account-balance",      demo::cmd_ws_account_balance,      "Account balance via WS API (auth)" },
    { "ws-order-place",          demo::cmd_ws_order_place,          "Place order via WS API <sym> <side> <type> [-q Q] [-p P] [-t TIF]" },
    { "ws-order-query",          demo::cmd_ws_order_query,          "Query order via WS API <symbol> <orderId>" },
    { "ws-order-modify",         demo::cmd_ws_order_modify,         "Modify order via WS API <sym> <side> <orderId> -q Q -p P" },
    { "ws-order-cancel",         demo::cmd_ws_order_cancel,         "Cancel order via WS API <symbol> <orderId>" },
    { "ws-position",             demo::cmd_ws_position,             "Position info via WS API [symbol]" },
    { "ws-algo-order-place",     demo::cmd_ws_algo_order_place,     "Place algo order via WS API <sym> <side> <type> <algo> -q Q" },
    { "ws-algo-order-cancel",    demo::cmd_ws_algo_order_cancel,    "Cancel algo order via WS API <algoId>" },
    { "ws-user-stream-start",    demo::cmd_ws_user_stream_start,    "Start user stream via WS API" },
    { "ws-user-stream-ping",     demo::cmd_ws_user_stream_ping,     "Ping user stream via WS API" },
    { "ws-user-stream-stop",     demo::cmd_ws_user_stream_stop,     "Stop user stream via WS API" },

    { "Market Streams", nullptr, {} },
    { "stream-aggregate-trade",  demo::cmd_stream_aggregate_trade,  "Aggregate trade stream <symbol>" },
    { "stream-book-ticker",      demo::cmd_stream_book_ticker,      "Book ticker stream <symbol>" },
    { "stream-mark-price",       demo::cmd_stream_mark_price,       "Mark price stream <symbol>" },
    { "stream-kline",            demo::cmd_stream_kline,            "Kline stream <symbol> <interval>" },
    { "stream-ticker",           demo::cmd_stream_ticker,           "24hr ticker stream <symbol>" },
    { "stream-mini-ticker",      demo::cmd_stream_mini_ticker,      "Mini ticker stream <symbol>" },
    { "stream-depth",            demo::cmd_stream_depth,            "Partial depth stream <symbol> [levels]" },
    { "stream-diff-depth",       demo::cmd_stream_diff_depth,       "Diff depth stream <symbol> [speed]" },
    { "stream-liquidation",      demo::cmd_stream_liquidation,      "Liquidation orders stream <symbol>" },
    { "stream-composite-index",  demo::cmd_stream_composite_index,  "Composite index stream <symbol>" },
    { "stream-asset-index",      demo::cmd_stream_asset_index,      "Asset index stream <symbol>" },
    { "stream-continuous-kline", demo::cmd_stream_continuous_kline,  "Continuous kline stream <pair> <interval>" },
    { "stream-rpi-diff-depth",   demo::cmd_stream_rpi_diff_depth,   "RPI diff depth stream <symbol>" },
    { "stream-all-book-tickers", demo::cmd_stream_all_book_tickers, "All book tickers stream" },
    { "stream-all-tickers",      demo::cmd_stream_all_tickers,      "All 24hr tickers stream" },
    { "stream-all-mini-tickers", demo::cmd_stream_all_mini_tickers, "All mini tickers stream" },
    { "stream-all-liquidations", demo::cmd_stream_all_liquidations, "All liquidation orders stream" },
    { "stream-all-mark-prices",  demo::cmd_stream_all_mark_prices,  "All mark prices stream" },
    { "stream-all-asset-index",  demo::cmd_stream_all_asset_index,  "All asset index stream" },
    { "stream-contract-info",    demo::cmd_stream_contract_info,    "Contract info stream" },
    { "stream-trading-session",  demo::cmd_stream_trading_session,  "Trading session stream" },

    { "User Data Streams", nullptr, {} },
    { "listen-key-start",        demo::cmd_listen_key_start,        "Start listen key (auth)" },
    { "listen-key-keepalive",    demo::cmd_listen_key_keepalive,    "Keepalive listen key (auth)" },
    { "listen-key-close",        demo::cmd_listen_key_close,        "Close listen key (auth)" },
    { "user-stream",             demo::cmd_user_stream,             "User data stream demo (auth)" },

    { "Order Book", nullptr, {} },
    { "order-book-live",         demo::cmd_order_book_live,         "Live order book <symbol> [depth]" },
    { "pipeline-order-book-live", demo::cmd_pipeline_order_book_live, "Pipeline order book (3 threads) <symbol> [depth]" },
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
              << "  -K, --secrets <source>      Secret source: libsecret:<profile> (default), env, systemd-creds:<dir>\n"
              << "  -L, --log-file <file>       Log to file\n"
              << "  -F, --file-loglevel <lvl>   File log level (trace/debug/info/warn/error/off)\n"
              << "  -O, --stdout-loglevel <lvl> Stdout log level (trace/debug/info/warn/error/off)\n"
              << "  -h, --help                  Print this help\n\n"
              << "Commands:\n";
    for (const auto& cmd : commands) {
        if (is_group(cmd)) {
            std::cout << '\n' << "  " << cmd.name << ":\n";
        } else {
            std::cout << "    " << cmd.name
                      << std::string(25 - std::min(cmd.name.size(), std::size_t{24}), ' ')
                      << cmd.help << '\n';
        }
    }
}

} // namespace

boost::cobalt::main co_main(int argc, char* argv[])
{
    if (argc < 2) { print_usage(argv[0]); co_return 1; }

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
        else if ((arg == "--secrets"      || arg == "-K") && i + 1 < argc) { demo::secrets_source     = argv[++i]; }
        else if ((arg == "--log-file"      || arg == "-L") && i + 1 < argc) { demo::log_file           = argv[++i]; }
        else if ((arg == "--file-loglevel" || arg == "-F") && i + 1 < argc) { demo::file_loglevel      = argv[++i]; }
        else if ((arg == "--stdout-loglevel" || arg == "-O") && i + 1 < argc) { demo::stdout_loglevel  = argv[++i]; }
        else if (arg == "--help" || arg == "-h") { print_usage(prog); co_return 0; }
        else { cmd_args.emplace_back(arg); }
    }

    if (cmd_args.empty()) { print_usage(prog); co_return 1; }

    demo::init_logging();

    // Set up stream recording if --record was specified.
    std::unique_ptr<binapi2::fapi::streams::spdlog_stream_recorder> recorder;
    if (auto rec_logger = spdlog::get("rec")) {
        recorder = std::make_unique<binapi2::fapi::streams::spdlog_stream_recorder>(4096);
        demo::record_buffer = &recorder->add_stream(
            binapi2::fapi::streams::sinks::spdlog_sink(rec_logger));
        recorder->start();
        spdlog::info("recording stream frames to {}", demo::record_file);
    }

    std::string_view cmd_name = cmd_args[0];
    spdlog::info("command: {}", cmd_name);

    // Remove the command name, pass only subcommand args.
    demo::args_t sub_args(cmd_args.begin() + 1, cmd_args.end());

    // Create config and load credentials.
    auto cfg = demo::make_config();
    co_await demo::async_load_secrets(cfg);
    binapi2::futures_usdm_api c(std::move(cfg));

    // Install SIGINT/SIGTERM handler for graceful shutdown.
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](boost::system::error_code ec, int sig) {
        if (ec) return; // cancelled
        spdlog::info("signal {} received, shutting down...", sig);
        ioc.stop();
    });

    int rc = 1;
    for (const auto& cmd : commands) {
        if (!is_group(cmd) && cmd.name == cmd_name) {
            rc = co_await cmd.fn(c, sub_args);
            break;
        }
    }

    if (rc == 1 && std::none_of(std::begin(commands), std::end(commands),
                                [&](const auto& cmd) { return !is_group(cmd) && cmd.name == cmd_name; })) {
        spdlog::error("unknown command: {}", cmd_name);
        print_usage(prog);
    }

    signals.cancel();
    if (recorder) recorder->stop();
    demo::shutdown_logging();
    co_return rc;
}
