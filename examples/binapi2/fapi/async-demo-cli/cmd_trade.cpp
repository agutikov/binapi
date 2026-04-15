// SPDX-License-Identifier: Apache-2.0
//
// Trade commands — authenticated order management and position configuration.

#include "cmd_trade.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

#include <sstream>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

template<typename Request>
CLI::App* add_noarg(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto* sub = parent.add_subcommand(name, desc);
    sub->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_trade(c, Request{}, sink);
        };
    });
    return sub;
}

template<typename Request>
CLI::App* add_symbol(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_trade(
                c, lib::make_symbol_request<Request>(*opts), sink);
        };
    });
    return sub;
}

template<typename Request>
CLI::App* add_optional_symbol(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            Request req;
            if (!opts->symbol.empty()) req.symbol = opts->symbol;
            co_return co_await lib::exec_trade(c, std::move(req), sink);
        };
    });
    return sub;
}

template<typename Request>
CLI::App* add_symbol_limit(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_limit_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->add_option("-l,--limit", opts->limit, "Result limit");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_trade(
                c, lib::make_symbol_limit_request<Request>(*opts), sink);
        };
    });
    return sub;
}

template<typename Request>
CLI::App* add_symbol_order_id(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_order_id_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol",  opts->symbol,   "Trading symbol")->required();
    sub->add_option("orderId", opts->order_id, "Order ID")->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_trade(
                c, lib::make_symbol_order_id_request<Request>(*opts), sink);
        };
    });
    return sub;
}

/// Bind the `<symbol> <side> <type> [-q] [-p] [-t]` options of a new-order
/// style request. Returns the shared opts struct (alive for the callback
/// lifetime via the caller's capture).
std::shared_ptr<lib::order_opts>
bind_new_order_opts(CLI::App& sub)
{
    auto opts = std::make_shared<lib::order_opts>();
    sub.add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub.add_option("side",   opts->side,   "Side (BUY|SELL)")->required();
    sub.add_option("type",   opts->type,   "Order type (LIMIT|MARKET|…)")->required();
    sub.add_option("-q,--quantity", opts->quantity, "Quantity");
    sub.add_option("-p,--price",    opts->price,    "Price (LIMIT only)");
    sub.add_option("-t,--tif",      opts->tif,      "Time in force (GTC|IOC|FOK)");
    return opts;
}

} // namespace

void register_cmd_trade(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Trade";

    // ── order placement ───────────────────────────────────────────────

    {
        auto* sub = app.add_subcommand("new-order", "Place order <sym> <side> <type> [-q Q] [-p P] [-t TIF]");
        sub->group(group);
        auto opts = bind_new_order_opts(*sub);
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                co_return co_await lib::exec_trade(
                    c, lib::make_order_request<types::new_order_request_t>(*opts), sink);
            };
        });
    }

    {
        auto* sub = app.add_subcommand("test-order", "Test order (validates, does not place)");
        sub->group(group);
        auto opts = bind_new_order_opts(*sub);
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                auto ord = lib::make_order_request<types::new_order_request_t>(*opts);
                types::test_order_request_t req;
                req.symbol = ord.symbol;
                req.side = ord.side;
                req.type = ord.type;
                req.quantity = ord.quantity;
                req.price = ord.price;
                req.timeInForce = ord.timeInForce;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::string symbol, side, quantity, price; std::uint64_t order_id = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("modify-order", "Modify order <sym> <side> <orderId> -q Q -p P");
        sub->group(group);
        sub->add_option("symbol",  opts->symbol,   "Trading symbol")->required();
        sub->add_option("side",    opts->side,     "Side (BUY|SELL)")->required();
        sub->add_option("orderId", opts->order_id, "Order ID")->required();
        sub->add_option("-q,--quantity", opts->quantity, "New quantity")->required();
        sub->add_option("-p,--price",    opts->price,    "New price")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::modify_order_request_t req;
                req.symbol = opts->symbol;
                req.side = lib::parse_enum<types::order_side_t>(opts->side);
                req.orderId = opts->order_id;
                req.quantity = types::decimal_t(opts->quantity);
                req.price = types::decimal_t(opts->price);
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    add_symbol_order_id<types::cancel_order_request_t>    (app, "cancel-order",     "Cancel order", sel)->group(group);
    add_symbol_order_id<types::query_order_request_t>     (app, "query-order",      "Query order", sel)->group(group);
    add_symbol_order_id<types::query_open_order_request_t>(app, "query-open-order", "Query open order", sel)->group(group);

    // cancel-multiple-orders: <symbol> <id1,id2,…>
    {
        struct opts_t { std::string symbol, ids_csv; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("cancel-multiple-orders", "Cancel orders <symbol> <id1,id2,…>");
        sub->group(group);
        sub->add_option("symbol", opts->symbol,  "Trading symbol")->required();
        sub->add_option("ids",    opts->ids_csv, "Comma-separated order IDs")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                auto rest = co_await c.create_rest_client();
                if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
                types::cancel_multiple_orders_request_t req;
                req.symbol = opts->symbol;
                std::vector<std::uint64_t> ids;
                std::istringstream ss(opts->ids_csv);
                std::string tok;
                while (std::getline(ss, tok, ','))
                    ids.push_back(std::stoull(tok));
                req.orderIdList = std::move(ids);
                auto r = co_await (*rest)->trade.async_cancel_batch_orders(req);
                if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
                if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
                    sink.on_response_json(*j);
                sink.on_done(0);
                co_return 0;
            };
        });
    }

    add_symbol<types::cancel_all_open_orders_request_t>(app, "cancel-all-orders", "Cancel all open orders", sel)->group(group);

    // auto-cancel: <symbol> <countdownMs>
    {
        struct opts_t { std::string symbol; std::uint64_t countdown = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("auto-cancel", "Auto-cancel <symbol> <countdownMs>");
        sub->group(group);
        sub->add_option("symbol",    opts->symbol,    "Trading symbol")->required();
        sub->add_option("countdown", opts->countdown, "Countdown (ms, 0 to cancel)")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::auto_cancel_request_t req;
                req.symbol = opts->symbol;
                req.countdownTime = opts->countdown;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    // ── queries ───────────────────────────────────────────────────────

    add_optional_symbol<types::all_open_orders_request_t>  (app, "open-orders",      "Open orders [symbol]", sel)->group(group);
    add_symbol_limit   <types::all_orders_request_t>       (app, "all-orders",       "All orders", sel)->group(group);
    add_optional_symbol<types::position_info_v3_request_t> (app, "position-info-v3", "Position info v3 [symbol]", sel)->group(group);
    add_optional_symbol<types::adl_quantile_request_t>     (app, "adl-quantile",     "ADL quantile [symbol]", sel)->group(group);

    // force-orders: [symbol] [--limit N]
    {
        struct opts_t { std::string symbol; std::optional<int> limit; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("force-orders", "Force orders [symbol] [--limit L]");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
        sub->add_option("-l,--limit", opts->limit, "Result limit");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::force_orders_request_t req;
                if (!opts->symbol.empty()) req.symbol = opts->symbol;
                req.limit = opts->limit;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    add_symbol_limit<types::account_trade_request_t>(app, "account-trades", "Account trades", sel)->group(group);

    // ── configuration changes ─────────────────────────────────────────

    {
        struct opts_t { std::string dual; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("change-position-mode", "Change position mode <true|false>");
        sub->group(group);
        sub->add_option("dual", opts->dual, "true|false")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::change_position_mode_request_t req;
                req.dualSidePosition = opts->dual;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::string multi; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("change-multi-assets-mode", "Change multi-assets mode <true|false>");
        sub->group(group);
        sub->add_option("multi", opts->multi, "true|false")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::change_multi_assets_mode_request_t req;
                req.multiAssetsMargin = opts->multi;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::string symbol; int leverage = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("change-leverage", "Change leverage <symbol> <leverage>");
        sub->group(group);
        sub->add_option("symbol",   opts->symbol,   "Trading symbol")->required();
        sub->add_option("leverage", opts->leverage, "Leverage (integer)")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::change_leverage_request_t req;
                req.symbol = opts->symbol;
                req.leverage = opts->leverage;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::string symbol, type; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("change-margin-type", "Change margin type <symbol> <ISOLATED|CROSSED>");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->add_option("type",   opts->type,   "ISOLATED|CROSSED")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::change_margin_type_request_t req;
                req.symbol = opts->symbol;
                req.marginType = opts->type;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::string symbol, amount; int type = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("modify-isolated-margin", "Modify isolated margin <sym> <amount> <1|2>");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->add_option("amount", opts->amount, "Amount")->required();
        sub->add_option("type",   opts->type,   "1=add, 2=reduce")
            ->required()->check(CLI::IsMember({ 1, 2 }));
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::modify_isolated_margin_request_t req;
                req.symbol = opts->symbol;
                req.amount = types::decimal_t(opts->amount);
                req.type = static_cast<types::delta_type_t>(opts->type);
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    add_symbol_limit<types::position_margin_history_request_t>(app, "position-margin-history", "Position margin history", sel)->group(group);

    // order-modify-history: <symbol> [--order-id N]
    {
        struct opts_t { std::string symbol; std::optional<std::uint64_t> order_id; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("order-modify-history", "Order modify history <symbol> [--order-id N]");
        sub->group(group);
        sub->add_option("symbol",        opts->symbol,   "Trading symbol")->required();
        sub->add_option("-o,--order-id", opts->order_id, "Order ID (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::order_modify_history_request_t req;
                req.symbol = opts->symbol;
                req.orderId = opts->order_id;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    // ── algo orders ───────────────────────────────────────────────────

    {
        struct opts_t { std::string symbol, side, type, algo, quantity, price; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("new-algo-order", "Place algo order <sym> <side> <type> <algo> -q Q [-p P]");
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
                types::new_algo_order_request_t req;
                req.symbol = opts->symbol;
                req.side = lib::parse_enum<types::order_side_t>(opts->side);
                req.type = lib::parse_enum<types::order_type_t>(opts->type);
                req.algoType = lib::parse_enum<types::algo_type_t>(opts->algo);
                req.quantity = types::decimal_t(opts->quantity);
                if (!opts->price.empty()) req.price = types::decimal_t(opts->price);
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::uint64_t algo_id = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("cancel-algo-order", "Cancel algo order <algoId>");
        sub->group(group);
        sub->add_option("algoId", opts->algo_id, "Algo ID")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::cancel_algo_order_request_t req;
                req.algoId = opts->algo_id;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    {
        struct opts_t { std::uint64_t algo_id = 0; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("query-algo-order", "Query algo order <algoId>");
        sub->group(group);
        sub->add_option("algoId", opts->algo_id, "Algo ID")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::query_algo_order_request_t req;
                req.algoId = opts->algo_id;
                co_return co_await lib::exec_trade(c, std::move(req), sink);
            };
        });
    }

    add_symbol_limit<types::all_algo_orders_request_t>       (app, "all-algo-orders",       "All algo orders", sel)->group(group);
    add_noarg       <types::open_algo_orders_request_t>      (app, "open-algo-orders",      "Open algo orders", sel)->group(group);
    add_noarg       <types::cancel_all_algo_orders_request_t>(app, "cancel-all-algo-orders","Cancel all algo orders", sel)->group(group);
    add_noarg       <types::tradfi_perps_request_t>          (app, "tradfi-perps",          "TradFi perps", sel)->group(group);
}

} // namespace demo
