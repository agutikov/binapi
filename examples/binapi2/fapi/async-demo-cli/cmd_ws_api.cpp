// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API commands — JSON-RPC over WebSocket.

#include "cmd_ws_api.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

template<typename Request>
CLI::App* add_ws_noarg_signed(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto* sub = parent.add_subcommand(name, desc);
    sub->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_ws_signed(c, Request{}, sink);
        };
    });
    return sub;
}

template<typename Request>
CLI::App* add_ws_symbol_order_id(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_order_id_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol",  opts->symbol,   "Trading symbol")->required();
    sub->add_option("orderId", opts->order_id, "Order ID")->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_ws_signed(
                c, lib::make_symbol_order_id_request<Request>(*opts), sink);
        };
    });
    return sub;
}

} // namespace

void register_cmd_ws_api(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "WebSocket API";

    // ── session ───────────────────────────────────────────────────────

    {
        auto* sub = app.add_subcommand("ws-logon", "WebSocket API session logon (auth)");
        sub->group(group);
        sub->callback([&sel] {
            sel.factory = [](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
                auto ws = co_await c.create_ws_api_client();
                if (!ws) { sink.on_error(ws.err); sink.on_done(1); co_return 1; }
                if (auto conn = co_await (*ws)->async_connect(); !conn) {
                    co_await (*ws)->async_close();
                    sink.on_error(conn.err); sink.on_done(1); co_return 1;
                }
                auto r = co_await (*ws)->async_session_logon();
                if (!r) {
                    co_await (*ws)->async_close();
                    sink.on_error(r.err); sink.on_done(1); co_return 1;
                }
                sink.on_info("session logon ok, status=" + std::to_string(r->status));
                if (r->result) {
                    if (auto j = glz::write<glz::opts{ .prettify = true }>(*r->result); j)
                        sink.on_response_json(*j);
                }
                co_await (*ws)->async_close();
                sink.on_done(0);
                co_return 0;
            };
        });
    }

    add_ws_noarg_signed<types::ws_account_status_request_t>   (app, "ws-account-status",    "Account status via WS API (auth)",    sel)->group(group);
    add_ws_noarg_signed<types::ws_account_status_v2_request_t>(app, "ws-account-status-v2", "Account status v2 via WS API (auth)", sel)->group(group);
    add_ws_noarg_signed<types::ws_account_balance_request_t>  (app, "ws-account-balance",   "Account balance via WS API (auth)",   sel)->group(group);

    // ── market data (public) ──────────────────────────────────────────

    {
        struct opts_t { std::string symbol; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("ws-book-ticker", "Book ticker via WS API [symbol]");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                if (opts->symbol.empty()) {
                    co_return co_await lib::exec_ws_public(
                        c, types::websocket_api_book_tickers_request_t{}, sink);
                }
                types::websocket_api_book_ticker_request_t req;
                req.symbol = opts->symbol;
                co_return co_await lib::exec_ws_public(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::string symbol; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("ws-price-ticker", "Price ticker via WS API [symbol]");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                if (opts->symbol.empty()) {
                    co_return co_await lib::exec_ws_public(
                        c, types::websocket_api_price_tickers_request_t{}, sink);
                }
                types::websocket_api_price_ticker_request_t req;
                req.symbol = opts->symbol;
                co_return co_await lib::exec_ws_public(c, std::move(req), sink);
            };
        });
    }

    // ── order management ──────────────────────────────────────────────

    {
        auto opts = std::make_shared<lib::order_opts>();
        auto* sub = app.add_subcommand("ws-order-place", "Place order via WS API");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->add_option("side",   opts->side,   "Side (BUY|SELL)")->required();
        sub->add_option("type",   opts->type,   "Order type (LIMIT|MARKET|…)")->required();
        sub->add_option("-q,--quantity", opts->quantity, "Quantity");
        sub->add_option("-p,--price",    opts->price,    "Price (LIMIT only)");
        sub->add_option("-t,--tif",      opts->tif,      "Time in force");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                co_return co_await lib::exec_ws_signed(
                    c, lib::make_order_request<types::websocket_api_order_place_request_t>(*opts), sink);
            };
        });
    }

    add_ws_symbol_order_id<types::websocket_api_order_query_request_t> (app, "ws-order-query",  "Query order via WS API",  sel)->group(group);
    add_ws_symbol_order_id<types::websocket_api_order_cancel_request_t>(app, "ws-order-cancel", "Cancel order via WS API", sel)->group(group);

    {
        struct opts_t { std::string symbol, side, quantity, price; std::uint64_t order_id = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("ws-order-modify", "Modify order via WS API");
        sub->group(group);
        sub->add_option("symbol",  opts->symbol,   "Trading symbol")->required();
        sub->add_option("side",    opts->side,     "Side (BUY|SELL)")->required();
        sub->add_option("orderId", opts->order_id, "Order ID")->required();
        sub->add_option("-q,--quantity", opts->quantity, "New quantity")->required();
        sub->add_option("-p,--price",    opts->price,    "New price")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::websocket_api_order_modify_request_t req;
                req.symbol = opts->symbol;
                req.side = lib::parse_enum<types::order_side_t>(opts->side);
                req.orderId = opts->order_id;
                req.quantity = types::decimal_t(opts->quantity);
                req.price = types::decimal_t(opts->price);
                co_return co_await lib::exec_ws_signed(c, std::move(req), sink);
            };
        });
    }

    // ws-position: [symbol]
    {
        struct opts_t { std::string symbol; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("ws-position", "Position info via WS API [symbol]");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::websocket_api_position_request_t req;
                if (!opts->symbol.empty()) req.symbol = opts->symbol;
                co_return co_await lib::exec_ws_signed(c, std::move(req), sink);
            };
        });
    }

    // ── algo orders ───────────────────────────────────────────────────

    {
        struct opts_t { std::string symbol, side, type, algo, quantity, price; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("ws-algo-order-place", "Place algo order via WS API");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->add_option("side",   opts->side,   "Side (BUY|SELL)")->required();
        sub->add_option("type",   opts->type,   "Order type")->required();
        sub->add_option("algo",   opts->algo,   "Algo type")->required();
        sub->add_option("-q,--quantity", opts->quantity, "Quantity")->required();
        sub->add_option("-p,--price",    opts->price,    "Price (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::websocket_api_algo_order_place_request_t req;
                req.symbol = opts->symbol;
                req.side = lib::parse_enum<types::order_side_t>(opts->side);
                req.type = lib::parse_enum<types::order_type_t>(opts->type);
                req.algoType = lib::parse_enum<types::algo_type_t>(opts->algo);
                req.quantity = types::decimal_t(opts->quantity);
                if (!opts->price.empty()) req.price = types::decimal_t(opts->price);
                co_return co_await lib::exec_ws_signed(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::uint64_t algo_id = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("ws-algo-order-cancel", "Cancel algo order via WS API");
        sub->group(group);
        sub->add_option("algoId", opts->algo_id, "Algo ID")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::websocket_api_algo_order_cancel_request_t req;
                req.algoId = opts->algo_id;
                co_return co_await lib::exec_ws_signed(c, std::move(req), sink);
            };
        });
    }

    // ── user data stream lifecycle (all signed, no args) ──────────────

    add_ws_noarg_signed<types::ws_user_data_stream_start_request_t>(app, "ws-user-stream-start", "Start user stream via WS API", sel)->group(group);
    add_ws_noarg_signed<types::ws_user_data_stream_ping_request_t> (app, "ws-user-stream-ping",  "Ping user stream via WS API",  sel)->group(group);
    add_ws_noarg_signed<types::ws_user_data_stream_stop_request_t> (app, "ws-user-stream-stop",  "Stop user stream via WS API",  sel)->group(group);
}

} // namespace demo
