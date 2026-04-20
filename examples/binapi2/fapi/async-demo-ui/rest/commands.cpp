// SPDX-License-Identifier: Apache-2.0
//
// Shared implementation: form-field visibility predicates, capture reset,
// small string helpers. Per-command registries live in their own TU.

#include "commands.hpp"

#include <sstream>

namespace demo_ui::rest {

// ── needs_* predicates ───────────────────────────────────────────
//
// Each answers: does this form_kind need the given input field?
// One predicate per field. The middle-column renderer walks these and
// only shows inputs whose predicate returns true.

bool needs_symbol(form_kind k)
{
    switch (k) {
        case form_kind::symbol:
        case form_kind::symbol_opt:
        case form_kind::symbol_limit:
        case form_kind::symbol_opt_limit:
        case form_kind::symbol_order_id:
        case form_kind::symbol_order_id_opt:
        case form_kind::kline:
        case form_kind::analytics:
        case form_kind::order_form:
        case form_kind::modify_order_form:
        case form_kind::algo_order_form:
        case form_kind::cancel_multi:
        case form_kind::auto_cancel_form:
        case form_kind::change_leverage_form:
        case form_kind::change_margin_type_form:
        case form_kind::modify_isolated_margin_form:
            return true;
        default:
            return false;
    }
}

bool needs_pair(form_kind k)
{
    return k == form_kind::pair || k == form_kind::pair_kline || k == form_kind::basis;
}

bool needs_limit(form_kind k)
{
    switch (k) {
        case form_kind::symbol_limit:
        case form_kind::symbol_opt_limit:
        case form_kind::kline:
        case form_kind::pair_kline:
        case form_kind::analytics:
        case form_kind::basis:
            return true;
        default:
            return false;
    }
}

bool needs_interval(form_kind k) { return k == form_kind::kline || k == form_kind::pair_kline; }
bool needs_period  (form_kind k) { return k == form_kind::analytics || k == form_kind::basis; }

bool needs_order_id(form_kind k)
{
    return k == form_kind::symbol_order_id || k == form_kind::symbol_order_id_opt
        || k == form_kind::modify_order_form;
}

bool needs_algo_id(form_kind k) { return k == form_kind::algo_id_form; }

bool needs_side(form_kind k)
{
    return k == form_kind::order_form || k == form_kind::modify_order_form
        || k == form_kind::algo_order_form;
}

bool needs_order_type(form_kind k) { return k == form_kind::order_form || k == form_kind::algo_order_form; }
bool needs_algo_type (form_kind k) { return k == form_kind::algo_order_form; }
bool needs_tif       (form_kind k) { return k == form_kind::order_form; }

bool needs_quantity(form_kind k)
{
    return k == form_kind::order_form || k == form_kind::modify_order_form
        || k == form_kind::algo_order_form;
}

bool needs_price(form_kind k)
{
    return k == form_kind::order_form || k == form_kind::modify_order_form
        || k == form_kind::algo_order_form;
}

bool needs_leverage   (form_kind k) { return k == form_kind::change_leverage_form; }
bool needs_margin_type(form_kind k) { return k == form_kind::change_margin_type_form; }
bool needs_amount     (form_kind k) { return k == form_kind::modify_isolated_margin_form; }
bool needs_delta_type (form_kind k) { return k == form_kind::modify_isolated_margin_form; }
bool needs_countdown  (form_kind k) { return k == form_kind::auto_cancel_form; }
bool needs_bool_flag  (form_kind k) { return k == form_kind::bool_toggle; }
bool needs_asset      (form_kind k) { return k == form_kind::asset_form; }
bool needs_ids_csv    (form_kind k) { return k == form_kind::cancel_multi; }
bool needs_start_time (form_kind k) { return k == form_kind::download_id_range; }
bool needs_end_time   (form_kind k) { return k == form_kind::download_id_range; }
bool needs_download_id(form_kind k) { return k == form_kind::download_link; }
bool needs_from_asset (form_kind k) { return k == form_kind::convert_quote_form; }
bool needs_to_asset   (form_kind k) { return k == form_kind::convert_quote_form; }
bool needs_from_amount(form_kind k) { return k == form_kind::convert_quote_form; }
bool needs_quote_id   (form_kind k) { return k == form_kind::quote_id_form; }
bool needs_convert_order_id(form_kind k) { return k == form_kind::convert_order_id_form; }

const char* bool_flag_label(form_kind k)
{
    // The bool_toggle form_kind is reused by several commands that each
    // name their boolean differently. Without context we fall back to
    // the generic label; per-command run_fn knows what field it maps to.
    (void)k;
    return "enable (true/false):";
}

// ── Reset helper ─────────────────────────────────────────────────

void reset_capture(request_capture& cap)
{
    {
        std::lock_guard lk(cap.mtx);
        cap.info_lines.clear();
        cap.request = capture_side{};
        cap.response = capture_side{};
        cap.error_message.clear();
        cap.http_status = 0;
        cap.binance_code = 0;
    }
    cap.state.store(request_capture::idle);
}

// ── CSV helper ───────────────────────────────────────────────────

std::vector<std::uint64_t> parse_u64_csv(std::string_view csv)
{
    std::vector<std::uint64_t> out;
    std::istringstream ss{ std::string{ csv } };
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        if (tok.empty()) continue;
        try { out.push_back(std::stoull(tok)); } catch (...) { /* skip */ }
    }
    return out;
}

} // namespace demo_ui::rest
