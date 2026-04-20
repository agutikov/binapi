// SPDX-License-Identifier: Apache-2.0
//
// Trade command registry — authenticated order management and position config.

#include "commands.hpp"

#include <binapi2/demo_commands/enum_parse.hpp>
#include <binapi2/fapi/types/trade.hpp>

namespace demo_ui::rest {

namespace {

// ── Order placement ──────────────────────────────────────────────

/// Fill the shared order-form fields (symbol/side/type/qty/price/tif) on
/// any request type that has the matching field names — covers
/// `new_order_request_t` and `test_order_request_t`.
template<typename Req>
Req build_order_req(const form_state& f)
{
    Req req;
    req.symbol = f.symbol;
    req.side = lib::parse_enum<types::order_side_t>(f.side);
    req.type = lib::parse_enum<types::order_type_t>(f.order_type);
    if (!f.quantity.empty()) req.quantity = parse_decimal(f.quantity);
    if (!f.price.empty())    req.price    = parse_decimal(f.price);
    if (!f.tif.empty())      req.timeInForce = lib::parse_enum<types::time_in_force_t>(f.tif);
    return req;
}

void cmd_new_order (const cmd_ctx& c) { run_trd(c, build_order_req<types::new_order_request_t>(c.form)); }
void cmd_test_order(const cmd_ctx& c) { run_trd(c, build_order_req<types::test_order_request_t>(c.form)); }

void cmd_modify_order(const cmd_ctx& c)
{
    types::modify_order_request_t req;
    req.symbol = c.form.symbol;
    req.side = lib::parse_enum<types::order_side_t>(c.form.side);
    req.orderId = parse_optional_u64(c.form.order_id);
    req.quantity = parse_decimal(c.form.quantity);
    req.price    = parse_decimal(c.form.price);
    run_trd(c, std::move(req));
}

// ── Symbol + orderId ─────────────────────────────────────────────

template<typename Req>
void run_symbol_order_id_trd(const cmd_ctx& c)
{
    Req req;
    req.symbol = c.form.symbol;
    req.orderId = parse_optional_u64(c.form.order_id);
    run_trd(c, std::move(req));
}
void cmd_cancel_order     (const cmd_ctx& c) { run_symbol_order_id_trd<types::cancel_order_request_t>(c); }
void cmd_query_order      (const cmd_ctx& c) { run_symbol_order_id_trd<types::query_order_request_t>(c); }
void cmd_query_open_order (const cmd_ctx& c) { run_symbol_order_id_trd<types::query_open_order_request_t>(c); }

// ── Cancel multiple (symbol + csv of ids) ────────────────────────
//
// `cancel_multiple_orders_request_t` has no endpoint_traits and goes through
// the trade service's bespoke `async_cancel_batch_orders` method instead of
// the generic pipeline. Matches the CLI's handling in cmd_trade.cpp.

boost::cobalt::task<void>
run_cancel_batch(worker& wrk,
                 std::shared_ptr<capture_sink> sink,
                 std::shared_ptr<request_capture> cap,
                 types::cancel_multiple_orders_request_t req)
{
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    auto r = co_await rest->trade.async_cancel_batch_orders(req);
    if (!r) {
        sink->on_error(r.err);
        sink->on_done(1);
        co_return;
    }
    if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
        sink->on_response_json(*j);
    sink->on_done(0);
}

void cmd_cancel_multiple_orders(const cmd_ctx& c)
{
    types::cancel_multiple_orders_request_t req;
    req.symbol = c.form.symbol;
    auto ids = parse_u64_csv(c.form.ids_csv);
    if (!ids.empty()) req.orderIdList = std::move(ids);

    reset_capture(*c.cap);
    auto sink = std::make_shared<capture_sink>(c.cap, c.wrk, c.state);
    prefill_request_json(*c.cap, req);
    boost::cobalt::spawn(c.wrk.io().get_executor(),
                         run_cancel_batch(c.wrk, std::move(sink), c.cap, std::move(req)),
                         boost::asio::use_future);
}

// ── Cancel all (symbol) ──────────────────────────────────────────

void cmd_cancel_all_orders(const cmd_ctx& c)
{
    types::cancel_all_open_orders_request_t req;
    req.symbol = c.form.symbol;
    run_trd(c, std::move(req));
}

// ── Auto-cancel (symbol + countdown ms) ──────────────────────────

void cmd_auto_cancel(const cmd_ctx& c)
{
    types::auto_cancel_request_t req;
    req.symbol = c.form.symbol;
    req.countdownTime = parse_u64(c.form.countdown);
    run_trd(c, std::move(req));
}

// ── Queries with optional symbol ─────────────────────────────────

template<typename Req>
void run_symbol_opt_trd(const cmd_ctx& c)
{
    Req req;
    if (!c.form.symbol.empty()) req.symbol = c.form.symbol;
    run_trd(c, std::move(req));
}
void cmd_open_orders      (const cmd_ctx& c) { run_symbol_opt_trd<types::all_open_orders_request_t>(c); }
void cmd_position_info_v3 (const cmd_ctx& c) { run_symbol_opt_trd<types::position_info_v3_request_t>(c); }
void cmd_adl_quantile     (const cmd_ctx& c) { run_symbol_opt_trd<types::adl_quantile_request_t>(c); }

void cmd_force_orders(const cmd_ctx& c)
{
    types::force_orders_request_t req;
    if (!c.form.symbol.empty()) req.symbol = c.form.symbol;
    req.limit = parse_optional_int(c.form.limit);
    run_trd(c, std::move(req));
}

// ── Symbol + optional limit ──────────────────────────────────────

template<typename Req>
void run_symbol_limit_trd(const cmd_ctx& c)
{
    Req req;
    req.symbol = c.form.symbol;
    req.limit = parse_optional_int(c.form.limit);
    run_trd(c, std::move(req));
}
void cmd_all_orders              (const cmd_ctx& c) { run_symbol_limit_trd<types::all_orders_request_t>(c); }
void cmd_account_trades          (const cmd_ctx& c) { run_symbol_limit_trd<types::account_trade_request_t>(c); }
void cmd_position_margin_history (const cmd_ctx& c) { run_symbol_limit_trd<types::position_margin_history_request_t>(c); }
void cmd_all_algo_orders         (const cmd_ctx& c) { run_symbol_limit_trd<types::all_algo_orders_request_t>(c); }

// ── Boolean toggles ──────────────────────────────────────────────

void cmd_change_position_mode(const cmd_ctx& c)
{
    types::change_position_mode_request_t req;
    req.dualSidePosition = c.form.bool_flag;
    run_trd(c, std::move(req));
}

void cmd_change_multi_assets_mode(const cmd_ctx& c)
{
    types::change_multi_assets_mode_request_t req;
    req.multiAssetsMargin = c.form.bool_flag;
    run_trd(c, std::move(req));
}

// ── Position management ──────────────────────────────────────────

void cmd_change_leverage(const cmd_ctx& c)
{
    types::change_leverage_request_t req;
    req.symbol = c.form.symbol;
    req.leverage = parse_int(c.form.leverage);
    run_trd(c, std::move(req));
}

void cmd_change_margin_type(const cmd_ctx& c)
{
    types::change_margin_type_request_t req;
    req.symbol = c.form.symbol;
    req.marginType = c.form.margin_type;
    run_trd(c, std::move(req));
}

void cmd_modify_isolated_margin(const cmd_ctx& c)
{
    types::modify_isolated_margin_request_t req;
    req.symbol = c.form.symbol;
    req.amount = parse_decimal(c.form.amount);
    int t = parse_int(c.form.delta_type);
    req.type = (t == 2) ? types::delta_type_t::reduce : types::delta_type_t::add;
    run_trd(c, std::move(req));
}

// ── Order modify history (symbol + optional orderId) ─────────────

void cmd_order_modify_history(const cmd_ctx& c)
{
    types::order_modify_history_request_t req;
    req.symbol = c.form.symbol;
    req.orderId = parse_optional_u64(c.form.order_id);
    run_trd(c, std::move(req));
}

// ── Algo orders ──────────────────────────────────────────────────

void cmd_new_algo_order(const cmd_ctx& c)
{
    types::new_algo_order_request_t req;
    req.symbol = c.form.symbol;
    req.side = lib::parse_enum<types::order_side_t>(c.form.side);
    req.type = lib::parse_enum<types::order_type_t>(c.form.order_type);
    req.algoType = lib::parse_enum<types::algo_type_t>(c.form.algo_type);
    if (!c.form.quantity.empty()) req.quantity = parse_decimal(c.form.quantity);
    if (!c.form.price.empty())    req.price    = parse_decimal(c.form.price);
    run_trd(c, std::move(req));
}

void cmd_cancel_algo_order(const cmd_ctx& c)
{
    types::cancel_algo_order_request_t req;
    req.algoId = parse_optional_u64(c.form.algo_id);
    run_trd(c, std::move(req));
}

void cmd_query_algo_order(const cmd_ctx& c)
{
    types::query_algo_order_request_t req;
    req.algoId = parse_optional_u64(c.form.algo_id);
    run_trd(c, std::move(req));
}

// ── No-args ──────────────────────────────────────────────────────

void cmd_open_algo_orders       (const cmd_ctx& c) { run_trd(c, types::open_algo_orders_request_t{}); }
void cmd_cancel_all_algo_orders (const cmd_ctx& c) { run_trd(c, types::cancel_all_algo_orders_request_t{}); }
void cmd_tradfi_perps           (const cmd_ctx& c) { run_trd(c, types::tradfi_perps_request_t{}); }

constexpr rest_command entries[] = {
    // Order placement
    { "new-order",    "Place order",                    command_group::trade, form_kind::order_form,        &cmd_new_order },
    { "test-order",   "Test order (validates only)",    command_group::trade, form_kind::order_form,        &cmd_test_order },
    { "modify-order", "Modify order",                   command_group::trade, form_kind::modify_order_form, &cmd_modify_order },

    // Cancel / query
    { "cancel-order",            "Cancel order",         command_group::trade, form_kind::symbol_order_id, &cmd_cancel_order },
    { "query-order",             "Query order",          command_group::trade, form_kind::symbol_order_id, &cmd_query_order },
    { "query-open-order",        "Query open order",     command_group::trade, form_kind::symbol_order_id, &cmd_query_open_order },
    { "cancel-multiple-orders",  "Cancel orders (csv)",  command_group::trade, form_kind::cancel_multi,    &cmd_cancel_multiple_orders },
    { "cancel-all-orders",       "Cancel all open orders", command_group::trade, form_kind::symbol,        &cmd_cancel_all_orders },
    { "auto-cancel",             "Auto-cancel dead-man",   command_group::trade, form_kind::auto_cancel_form, &cmd_auto_cancel },

    // Queries
    { "open-orders",      "Open orders [symbol]",          command_group::trade, form_kind::symbol_opt,       &cmd_open_orders },
    { "all-orders",       "All orders",                    command_group::trade, form_kind::symbol_limit,     &cmd_all_orders },
    { "position-info-v3", "Position info v3 [symbol]",     command_group::trade, form_kind::symbol_opt,       &cmd_position_info_v3 },
    { "adl-quantile",     "ADL quantile [symbol]",         command_group::trade, form_kind::symbol_opt,       &cmd_adl_quantile },
    { "force-orders",     "Force orders [symbol] [limit]", command_group::trade, form_kind::symbol_opt_limit, &cmd_force_orders },
    { "account-trades",   "Account trades",                command_group::trade, form_kind::symbol_limit,     &cmd_account_trades },

    // Configuration changes
    { "change-position-mode",     "Change position mode",     command_group::trade, form_kind::bool_toggle,              &cmd_change_position_mode },
    { "change-multi-assets-mode", "Change multi-assets mode", command_group::trade, form_kind::bool_toggle,              &cmd_change_multi_assets_mode },
    { "change-leverage",          "Change leverage",          command_group::trade, form_kind::change_leverage_form,     &cmd_change_leverage },
    { "change-margin-type",       "Change margin type",       command_group::trade, form_kind::change_margin_type_form,  &cmd_change_margin_type },
    { "modify-isolated-margin",   "Modify isolated margin",   command_group::trade, form_kind::modify_isolated_margin_form, &cmd_modify_isolated_margin },

    // History
    { "position-margin-history", "Position margin history", command_group::trade, form_kind::symbol_limit,          &cmd_position_margin_history },
    { "order-modify-history",    "Order modify history",    command_group::trade, form_kind::symbol_order_id_opt,   &cmd_order_modify_history },

    // Algo orders
    { "new-algo-order",         "Place algo order",      command_group::trade, form_kind::algo_order_form, &cmd_new_algo_order },
    { "cancel-algo-order",      "Cancel algo order",     command_group::trade, form_kind::algo_id_form,    &cmd_cancel_algo_order },
    { "query-algo-order",       "Query algo order",      command_group::trade, form_kind::algo_id_form,    &cmd_query_algo_order },
    { "all-algo-orders",        "All algo orders",       command_group::trade, form_kind::symbol_limit,    &cmd_all_algo_orders },
    { "open-algo-orders",       "Open algo orders",      command_group::trade, form_kind::no_args,         &cmd_open_algo_orders },
    { "cancel-all-algo-orders", "Cancel all algo orders",command_group::trade, form_kind::no_args,         &cmd_cancel_all_algo_orders },
    { "tradfi-perps",           "TradFi perps",          command_group::trade, form_kind::no_args,         &cmd_tradfi_perps },
};

} // namespace

std::span<const rest_command> trade_commands()
{
    return { entries, sizeof(entries) / sizeof(entries[0]) };
}

} // namespace demo_ui::rest
