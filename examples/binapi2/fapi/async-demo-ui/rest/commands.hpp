// SPDX-License-Identifier: Apache-2.0
//
// Shared types for the REST-tab command registries.
//
// Each per-group translation unit (commands_market_data.cpp / account.cpp /
// trade.cpp / convert.cpp) builds a `std::span<const rest_command>` and
// exposes it here. The view in `views/rest_view.cpp` aggregates the four
// spans into one flat menu.
//
// Every `cmd_*` function reads the fields its `form_kind` declares, builds
// a typed request, and hands it to `run_cmd<Req>` which captures, prefills,
// and spawns a cobalt task.

#pragma once

#include "../app_state.hpp"
#include "../util/capture_sink.hpp"
#include "../util/request_capture.hpp"
#include "../views/response_pane.hpp"
#include "../worker.hpp"

#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/fapi/rest/client.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <charconv>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace demo_ui::rest {

namespace lib   = binapi2::demo;
namespace types = binapi2::fapi::types;

// ── Form state ────────────────────────────────────────────────────
//
// One flat bag of strings backing every input widget. Commands read only
// the fields their `form_kind` advertises; untouched fields keep whatever
// the user last typed.

struct form_state
{
    // Common
    std::string symbol   = "BTCUSDT";
    std::string pair     = "BTCUSDT";
    std::string limit    = "";
    std::string interval = "1m";
    std::string period   = "5m";

    // Order / trade
    std::string order_id   = "";
    std::string algo_id    = "";
    std::string side       = "BUY";
    std::string order_type = "LIMIT";
    std::string algo_type  = "VP";
    std::string tif        = "GTC";
    std::string quantity   = "";
    std::string price      = "";

    // Position / margin config
    std::string leverage    = "10";
    std::string margin_type = "ISOLATED";
    std::string amount      = "";
    std::string delta_type  = "1";          // 1=add, 2=reduce
    std::string countdown   = "0";

    // Shared yes/no toggles (content depends on command — label differs).
    std::string bool_flag = "false";

    // Misc
    std::string asset       = "USDT";
    std::string ids_csv     = "";
    std::string start_time  = "0";
    std::string end_time    = "0";
    std::string download_id = "";

    // Convert
    std::string from_asset       = "USDT";
    std::string to_asset         = "BTC";
    std::string from_amount      = "";
    std::string quote_id         = "";
    std::string convert_order_id = "";
};

/// Every concrete parameter shape used by at least one REST command.
/// Most are shared between groups; the ones prefixed with `ord_` / `algo_`
/// are trade-specific.
enum class form_kind
{
    no_args,
    symbol,
    symbol_opt,
    symbol_limit,
    pair,
    kline,
    pair_kline,
    analytics,
    basis,
    symbol_opt_limit,      // optional symbol + optional limit
    symbol_order_id,       // required symbol + required orderId
    symbol_order_id_opt,   // required symbol + optional orderId
    order_form,            // new-order / test-order
    modify_order_form,     // modify-order
    algo_order_form,       // new-algo-order
    algo_id_form,          // cancel/query-algo-order
    cancel_multi,          // symbol + ids csv
    auto_cancel_form,      // symbol + countdown
    bool_toggle,           // single true/false
    change_leverage_form,  // symbol + leverage
    change_margin_type_form,
    modify_isolated_margin_form,
    asset_form,            // pm-account-info
    download_id_range,     // start_time + end_time
    download_link,         // download_id
    convert_quote_form,    // from + to + amount
    quote_id_form,
    convert_order_id_form,
};

/// Which command-group does this entry belong to? Drives the group header
/// in the menu and the "auth required" flag on the Run action.
enum class command_group
{
    market_data,
    account,
    trade,
    convert,
};

// ── Form-kind → field visibility ──────────────────────────────────
//
// Each needs_* predicate answers: "does this form_kind use this field?"
// The middle column renders an input for every field whose predicate is
// true. Adding a new form_kind = extending these predicates.

bool needs_symbol       (form_kind k);
bool needs_pair         (form_kind k);
bool needs_limit        (form_kind k);
bool needs_interval     (form_kind k);
bool needs_period       (form_kind k);
bool needs_order_id     (form_kind k);
bool needs_algo_id      (form_kind k);
bool needs_side         (form_kind k);
bool needs_order_type   (form_kind k);
bool needs_algo_type    (form_kind k);
bool needs_tif          (form_kind k);
bool needs_quantity     (form_kind k);
bool needs_price        (form_kind k);
bool needs_leverage     (form_kind k);
bool needs_margin_type  (form_kind k);
bool needs_amount       (form_kind k);
bool needs_delta_type   (form_kind k);
bool needs_countdown    (form_kind k);
bool needs_bool_flag    (form_kind k);
bool needs_asset        (form_kind k);
bool needs_ids_csv      (form_kind k);
bool needs_start_time   (form_kind k);
bool needs_end_time     (form_kind k);
bool needs_download_id  (form_kind k);
bool needs_from_asset   (form_kind k);
bool needs_to_asset     (form_kind k);
bool needs_from_amount  (form_kind k);
bool needs_quote_id     (form_kind k);
bool needs_convert_order_id(form_kind k);

/// Human-readable label for the bool_flag field — changes per command.
const char* bool_flag_label(form_kind k);

// ── Parsing helpers ───────────────────────────────────────────────

inline std::optional<int> parse_optional_int(std::string_view s)
{
    if (s.empty()) return std::nullopt;
    int v = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), v);
    if (r.ec != std::errc{}) return std::nullopt;
    return v;
}

inline std::optional<std::uint64_t> parse_optional_u64(std::string_view s)
{
    if (s.empty()) return std::nullopt;
    std::uint64_t v = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), v);
    if (r.ec != std::errc{}) return std::nullopt;
    return v;
}

inline std::uint64_t parse_u64(std::string_view s)
{
    std::uint64_t v = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), v);
    if (r.ec != std::errc{}) return 0;
    return v;
}

inline int parse_int(std::string_view s)
{
    int v = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), v);
    if (r.ec != std::errc{}) return 0;
    return v;
}

/// Parse a decimal-as-string. Empty input returns an empty (zero) decimal.
inline types::decimal_t parse_decimal(std::string_view s)
{
    return s.empty() ? types::decimal_t{} : types::decimal_t(std::string{ s });
}

/// Parse a CSV list of unsigned integers (used by cancel-multiple-orders).
std::vector<std::uint64_t> parse_u64_csv(std::string_view csv);

// ── Capture plumbing ──────────────────────────────────────────────

void reset_capture(request_capture& cap);

// ── Per-service generic coroutines ────────────────────────────────
//
// Each templated free function instantiates once per Request type.
// Templates are explicitly allowed here — the instantiation is a free
// function so it satisfies `feedback_cobalt_lambda_lifetime.md`.

template<typename Request>
boost::cobalt::task<void>
run_market_data(worker& wrk,
                std::shared_ptr<capture_sink> sink,
                std::shared_ptr<request_capture> cap,
                Request req)
{
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    co_await lib::exec_market_data(*rest, std::move(req), *sink);
}

template<typename Request>
boost::cobalt::task<void>
run_account(worker& wrk,
            std::shared_ptr<capture_sink> sink,
            std::shared_ptr<request_capture> cap,
            Request req)
{
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    co_await lib::exec_account(*rest, std::move(req), *sink);
}

template<typename Request>
boost::cobalt::task<void>
run_trade(worker& wrk,
          std::shared_ptr<capture_sink> sink,
          std::shared_ptr<request_capture> cap,
          Request req)
{
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    co_await lib::exec_trade(*rest, std::move(req), *sink);
}

/// convert_service has no generic `async_execute`; we dispatch through
/// the pipeline directly (see exec.hpp note).
template<typename Request>
boost::cobalt::task<void>
run_convert(worker& wrk,
            std::shared_ptr<capture_sink> sink,
            std::shared_ptr<request_capture> cap,
            Request req)
{
    active_capture_guard guard(wrk, cap.get());
    auto* rest = co_await wrk.acquire_rest_client(*sink);
    if (!rest) co_return;
    auto r = co_await rest->rest_pipeline().async_execute(req);
    if (!r) {
        sink->on_error(r.err);
        sink->on_done(1);
        co_return;
    }
    if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
        sink->on_response_json(*j);
    sink->on_done(0);
}

// ── cmd_ctx + run_cmd dispatch ────────────────────────────────────

struct cmd_ctx
{
    worker& wrk;
    app_state& state;
    std::shared_ptr<request_capture> cap;
    const form_state& form;
};

/// Generic "build request was already done — capture, prefill, spawn".
/// `SpawnFn` is one of the `run_market_data / run_account / run_trade /
/// run_convert` templates above.
template<typename Request, auto SpawnFn>
void spawn_with(const cmd_ctx& c, Request req)
{
    reset_capture(*c.cap);
    auto sink = std::make_shared<capture_sink>(c.cap, c.wrk, c.state);
    prefill_request_json(*c.cap, req);
    boost::cobalt::spawn(c.wrk.io().get_executor(),
                         SpawnFn(c.wrk, std::move(sink), c.cap, std::move(req)),
                         boost::asio::use_future);
}

template<typename Request>
void run_market(const cmd_ctx& c, Request req)
{
    spawn_with<Request, &run_market_data<Request>>(c, std::move(req));
}

template<typename Request>
void run_acct(const cmd_ctx& c, Request req)
{
    spawn_with<Request, &run_account<Request>>(c, std::move(req));
}

template<typename Request>
void run_trd(const cmd_ctx& c, Request req)
{
    spawn_with<Request, &run_trade<Request>>(c, std::move(req));
}

template<typename Request>
void run_conv(const cmd_ctx& c, Request req)
{
    spawn_with<Request, &run_convert<Request>>(c, std::move(req));
}

// ── Command registry entry ────────────────────────────────────────

struct rest_command
{
    const char* name;
    const char* description;
    command_group group;
    form_kind form;
    void (*run)(const cmd_ctx&);
};

// Per-group registries. Each TU defines its span and extern-declares it here.
std::span<const rest_command> market_data_commands();
std::span<const rest_command> account_commands();
std::span<const rest_command> trade_commands();
std::span<const rest_command> convert_commands();

inline bool requires_auth(command_group g)
{
    return g != command_group::market_data;
}

} // namespace demo_ui::rest
