// SPDX-License-Identifier: Apache-2.0
//
// REST tab.
//
// Aggregates the four per-group command registries (market_data / account /
// trade / convert) from `rest/commands_*.cpp`, renders them as a collapsible
// scrollable menu, and gates Run on `credentials_loaded` for auth-required
// commands.
//
// The menu has two layers:
//
//   * `agg->entries` — immutable flat list of header + command rows, built
//     once at construction.
//   * `visible_titles` / `visible_map` — rebuilt whenever the user toggles
//     a group's collapsed state. `visible_map[i]` is the index into
//     `agg->entries` of the row currently shown at menu position i.
//
// Menu holds a raw pointer to `visible_titles` (see `feedback_ftxui_model_
// lifetime.md`); the Renderer/CatchEvent lambdas capture its shared_ptr.
//
// Form widgets are rendered conditionally per the selected command's
// `form_kind` via `Maybe(...)`, so hidden inputs drop out of focus
// traversal.

#include "../app_state.hpp"
#include "../rest/commands.hpp"
#include "../util/request_capture.hpp"
#include "../util/wrap_text.hpp"
#include "../worker.hpp"
#include "response_pane.hpp"
#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <spdlog/spdlog.h>

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;

namespace {

namespace R = demo_ui::rest;

const char* group_label(R::command_group g)
{
    switch (g) {
        case R::command_group::market_data: return "Market Data";
        case R::command_group::account:     return "Account";
        case R::command_group::trade:       return "Trade";
        case R::command_group::convert:     return "Convert";
        case R::command_group::ws_public:
        case R::command_group::ws_signed:   return "WebSocket API";
    }
    return "";
}

struct entry
{
    bool              is_header;
    R::command_group  group;
    int               cmd_idx;   // -1 for headers; else index into cmds
    std::string       name;      // bare name (header renders with arrow prefix)
};

struct aggregated
{
    std::vector<R::rest_command> cmds;
    std::vector<entry>           entries;
};

aggregated build_registry()
{
    aggregated agg;
    auto push_group = [&](R::command_group g, std::span<const R::rest_command> group_cmds) {
        if (group_cmds.empty()) return;
        agg.entries.push_back({ true, g, -1, group_label(g) });
        for (const auto& c : group_cmds) {
            int ci = static_cast<int>(agg.cmds.size());
            agg.cmds.push_back(c);
            agg.entries.push_back({ false, g, ci, c.name });
        }
    };
    push_group(R::command_group::market_data, R::market_data_commands());
    push_group(R::command_group::account,     R::account_commands());
    push_group(R::command_group::trade,       R::trade_commands());
    push_group(R::command_group::convert,     R::convert_commands());
    return agg;
}

} // namespace

tab_view make_rest_view(app_state& state, worker& wrk)
{
    spdlog::debug("make_rest_view: entering");

    auto agg = std::make_shared<aggregated>(build_registry());

    // One capture per command, indexed by cmd_idx (stable across rebuilds).
    // Header rows share a single dummy capture — never actually rendered
    // because the middle panel short-circuits on header selection.
    auto caps = std::make_shared<std::vector<std::shared_ptr<request_capture>>>();
    caps->reserve(agg->cmds.size());
    for (std::size_t i = 0; i < agg->cmds.size(); ++i)
        caps->emplace_back(std::make_shared<request_capture>());
    auto dummy_cap = std::make_shared<request_capture>();

    // Collapsed state per group — all expanded initially.
    auto collapsed = std::make_shared<std::array<bool, 4>>(
        std::array<bool, 4>{ false, false, false, false });

    // Visible-menu state — rebuilt on every collapse toggle.
    auto visible_titles = std::make_shared<std::vector<std::string>>();
    auto visible_map    = std::make_shared<std::vector<std::size_t>>();
    auto menu_index     = std::make_shared<int>(0);
    auto form           = std::make_shared<R::form_state>();

    // Rebuild `visible_titles` + `visible_map` from `agg->entries` and
    // `collapsed`. Tries to preserve the selection across the rebuild: if
    // the previously-selected entry is still visible, menu_index is moved
    // to it; otherwise we fall back to that entry's group header.
    std::function<void()> rebuild = [agg, collapsed, visible_titles,
                                     visible_map, menu_index] {
        // Remember which entry was selected.
        int prev_entry = -1;
        if (auto mi = static_cast<std::size_t>(*menu_index); mi < visible_map->size())
            prev_entry = static_cast<int>((*visible_map)[mi]);

        visible_titles->clear();
        visible_map->clear();
        for (std::size_t i = 0; i < agg->entries.size(); ++i) {
            const auto& e = agg->entries[i];
            const bool col = (*collapsed)[static_cast<std::size_t>(e.group)];
            if (e.is_header) {
                const char* arrow = col ? "▶ " : "▼ ";
                visible_titles->push_back(std::string{ arrow } + e.name);
                visible_map->push_back(i);
            } else if (!col) {
                visible_titles->push_back(std::string{ "  " } + e.name);
                visible_map->push_back(i);
            }
        }

        // Restore menu_index.
        int new_idx = 0;
        if (prev_entry >= 0) {
            bool found = false;
            for (std::size_t i = 0; i < visible_map->size(); ++i) {
                if (static_cast<int>((*visible_map)[i]) == prev_entry) {
                    new_idx = static_cast<int>(i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Entry now hidden — snap to its group header.
                auto g = agg->entries[static_cast<std::size_t>(prev_entry)].group;
                for (std::size_t i = 0; i < visible_map->size(); ++i) {
                    const auto& e = agg->entries[(*visible_map)[i]];
                    if (e.is_header && e.group == g) {
                        new_idx = static_cast<int>(i);
                        break;
                    }
                }
            }
        }
        *menu_index = new_idx;
    };
    rebuild();
    // Nudge selection off the first header onto the first command so the
    // form fields show up immediately.
    if (visible_map->size() > 1) *menu_index = 1;

    // Lookup helpers.
    auto selected_entry = [agg, visible_map, menu_index]() -> const entry* {
        auto mi = static_cast<std::size_t>(*menu_index);
        if (mi >= visible_map->size()) return nullptr;
        auto ei = (*visible_map)[mi];
        if (ei >= agg->entries.size()) return nullptr;
        return &agg->entries[ei];
    };

    auto selected_cmd = [agg, selected_entry]() -> const R::rest_command* {
        const entry* e = selected_entry();
        if (!e || e->is_header) return nullptr;
        return &agg->cmds[static_cast<std::size_t>(e->cmd_idx)];
    };

    auto selected_form = [selected_cmd]() -> R::form_kind {
        if (auto* c = selected_cmd()) return c->form;
        return R::form_kind::no_args;
    };

    auto run_blocked_reason = [&state, selected_cmd]() -> const char* {
        auto* c = selected_cmd();
        if (!c) return "select a command";
        if (R::requires_auth(c->group) && !state.credentials_loaded.load())
            return "credentials required";
        return nullptr;
    };

    // Menu points at `visible_titles`; its shared_ptr is captured by the
    // CatchEvent and Renderer lambdas (see file header).
    auto cmd_menu = Menu(visible_titles.get(), menu_index.get());

    auto cmd_menu_with_enter = CatchEvent(cmd_menu,
        [&wrk, &state, caps, dummy_cap, menu_index, form, agg,
         collapsed, visible_titles, visible_map,
         run_blocked_reason, selected_entry, rebuild](Event e) {
            (void)dummy_cap; (void)visible_titles; (void)visible_map;
            if (e != Event::Return) return false;
            const entry* ent = selected_entry();
            if (!ent) return true;
            if (ent->is_header) {
                auto g = static_cast<std::size_t>(ent->group);
                (*collapsed)[g] = !(*collapsed)[g];
                rebuild();
                return true;
            }
            if (const char* reason = run_blocked_reason()) {
                spdlog::info("rest_view: run blocked — {}", reason);
                return true;
            }
            const auto& cmd = agg->cmds[static_cast<std::size_t>(ent->cmd_idx)];
            R::cmd_ctx ctx{ wrk, state,
                            (*caps)[static_cast<std::size_t>(ent->cmd_idx)],
                            *form };
            cmd.run(ctx);
            return true;
        });

    // ── Input widgets. Each Maybe-gated on the selected form_kind. ───

    auto make_input = [](std::string* s, const char* placeholder) {
        return Input(s, placeholder);
    };

    auto sym_in    = make_input(&form->symbol,     "symbol");
    auto pair_in   = make_input(&form->pair,       "pair");
    auto lim_in    = make_input(&form->limit,      "limit");
    auto ival_in   = make_input(&form->interval,   "interval (1m, 5m, 1h, …)");
    auto per_in    = make_input(&form->period,     "period (5m, 15m, 1h, …)");
    auto oid_in    = make_input(&form->order_id,   "orderId");
    auto aid_in    = make_input(&form->algo_id,    "algoId");
    auto side_in   = make_input(&form->side,       "side (BUY|SELL)");
    auto otype_in  = make_input(&form->order_type, "type (LIMIT|MARKET|…)");
    auto atype_in  = make_input(&form->algo_type,  "algoType (VP|TWAP)");
    auto tif_in    = make_input(&form->tif,        "tif (GTC|IOC|FOK)");
    auto qty_in    = make_input(&form->quantity,   "quantity");
    auto price_in  = make_input(&form->price,      "price");
    auto lev_in    = make_input(&form->leverage,   "leverage");
    auto mtype_in  = make_input(&form->margin_type,"ISOLATED|CROSSED");
    auto amt_in    = make_input(&form->amount,     "amount");
    auto dtype_in  = make_input(&form->delta_type, "1=add 2=reduce");
    auto cntdn_in  = make_input(&form->countdown,  "countdown (ms, 0 to cancel)");
    auto bool_in   = make_input(&form->bool_flag,  "true|false");
    auto asset_in  = make_input(&form->asset,      "asset");
    auto ids_in    = make_input(&form->ids_csv,    "id1,id2,…");
    auto stime_in  = make_input(&form->start_time, "startTime (ms)");
    auto etime_in  = make_input(&form->end_time,   "endTime (ms)");
    auto dlid_in   = make_input(&form->download_id,"downloadId");
    auto from_in   = make_input(&form->from_asset, "from asset");
    auto to_in     = make_input(&form->to_asset,   "to asset");
    auto famt_in   = make_input(&form->from_amount,"from amount");
    auto quote_in  = make_input(&form->quote_id,   "quoteId");
    auto coid_in   = make_input(&form->convert_order_id, "orderId");

    auto maybe = [selected_form](auto widget, auto needs_fn) {
        return widget | Maybe([selected_form, needs_fn] { return needs_fn(selected_form()); });
    };

    auto sym_m    = maybe(sym_in,    R::needs_symbol);
    auto pair_m   = maybe(pair_in,   R::needs_pair);
    auto lim_m    = maybe(lim_in,    R::needs_limit);
    auto ival_m   = maybe(ival_in,   R::needs_interval);
    auto per_m    = maybe(per_in,    R::needs_period);
    auto oid_m    = maybe(oid_in,    R::needs_order_id);
    auto aid_m    = maybe(aid_in,    R::needs_algo_id);
    auto side_m   = maybe(side_in,   R::needs_side);
    auto otype_m  = maybe(otype_in,  R::needs_order_type);
    auto atype_m  = maybe(atype_in,  R::needs_algo_type);
    auto tif_m    = maybe(tif_in,    R::needs_tif);
    auto qty_m    = maybe(qty_in,    R::needs_quantity);
    auto price_m  = maybe(price_in,  R::needs_price);
    auto lev_m    = maybe(lev_in,    R::needs_leverage);
    auto mtype_m  = maybe(mtype_in,  R::needs_margin_type);
    auto amt_m    = maybe(amt_in,    R::needs_amount);
    auto dtype_m  = maybe(dtype_in,  R::needs_delta_type);
    auto cntdn_m  = maybe(cntdn_in,  R::needs_countdown);
    auto bool_m   = maybe(bool_in,   R::needs_bool_flag);
    auto asset_m  = maybe(asset_in,  R::needs_asset);
    auto ids_m    = maybe(ids_in,    R::needs_ids_csv);
    auto stime_m  = maybe(stime_in,  R::needs_start_time);
    auto etime_m  = maybe(etime_in,  R::needs_end_time);
    auto dlid_m   = maybe(dlid_in,   R::needs_download_id);
    auto from_m   = maybe(from_in,   R::needs_from_asset);
    auto to_m     = maybe(to_in,     R::needs_to_asset);
    auto famt_m   = maybe(famt_in,   R::needs_from_amount);
    auto quote_m  = maybe(quote_in,  R::needs_quote_id);
    auto coid_m   = maybe(coid_in,   R::needs_convert_order_id);

    auto rr_pane = make_request_response_pane(
        [agg, caps, dummy_cap, selected_entry]() -> std::shared_ptr<request_capture> {
            const entry* e = selected_entry();
            if (!e || e->is_header) return dummy_cap;
            return (*caps)[static_cast<std::size_t>(e->cmd_idx)];
        });

    // Form inputs sit in a Vertical container so ↑/↓ steps between them.
    // The outer Horizontal container then only sees three zones —
    // menu, form, pane — and →/← walks between those.
    auto form_container = Container::Vertical({
        sym_m, pair_m,
        ival_m, per_m,
        side_m, otype_m, atype_m, tif_m,
        qty_m, price_m,
        oid_m, aid_m,
        lev_m, mtype_m, amt_m, dtype_m,
        cntdn_m, bool_m, asset_m, ids_m,
        stime_m, etime_m, dlid_m,
        from_m, to_m, famt_m, quote_m, coid_m,
        lim_m,
    });

    auto root = Container::Horizontal({
        cmd_menu_with_enter,
        form_container,
        rr_pane,
    });

    // Initial focus on the menu — so the bottom keybar reflects the
    // active zone correctly from the first render, not the Container's
    // no-focus fallback.
    cmd_menu_with_enter->TakeFocus();

    auto component = Renderer(root, [cmd_menu_with_enter, rr_pane, agg, menu_index, caps,
                           dummy_cap, visible_titles, visible_map,
                           selected_cmd, selected_entry, run_blocked_reason,
                           sym_m, pair_m, lim_m, ival_m, per_m, oid_m, aid_m,
                           side_m, otype_m, atype_m, tif_m, qty_m, price_m,
                           lev_m, mtype_m, amt_m, dtype_m, cntdn_m, bool_m,
                           asset_m, ids_m, stime_m, etime_m, dlid_m,
                           from_m, to_m, famt_m, quote_m, coid_m] {
        (void)visible_titles; (void)visible_map; (void)dummy_cap;
        const auto* cmd = selected_cmd();
        const char* blocked = run_blocked_reason();

        Elements mid_rows;
        if (!cmd) {
            const entry* e = selected_entry();
            if (e && e->is_header) {
                mid_rows.push_back(text(e->name) | bold);
                mid_rows.push_back(text("(group header — Enter to collapse/expand)") | dim);
            } else {
                mid_rows.push_back(text("(no selection)") | dim);
            }
        } else {
            mid_rows.push_back(text(cmd->name) | bold);
            mid_rows.push_back(text(cmd->description) | dim);
            const auto fk = cmd->form;
            mid_rows.push_back(separator());

            auto labeled = [](const char* label, Element field) {
                return hbox({ text(label) | dim | size(WIDTH, EQUAL, 14),
                              field | border | flex });
            };

            if (R::needs_symbol(fk))       mid_rows.push_back(labeled("symbol:",    sym_m->Render()));
            if (R::needs_pair(fk))         mid_rows.push_back(labeled("pair:",      pair_m->Render()));
            if (R::needs_interval(fk))     mid_rows.push_back(labeled("interval:",  ival_m->Render()));
            if (R::needs_period(fk))       mid_rows.push_back(labeled("period:",    per_m->Render()));
            if (R::needs_side(fk))         mid_rows.push_back(labeled("side:",      side_m->Render()));
            if (R::needs_order_type(fk))   mid_rows.push_back(labeled("type:",      otype_m->Render()));
            if (R::needs_algo_type(fk))    mid_rows.push_back(labeled("algoType:",  atype_m->Render()));
            if (R::needs_tif(fk))          mid_rows.push_back(labeled("tif:",       tif_m->Render()));
            if (R::needs_quantity(fk))     mid_rows.push_back(labeled("quantity:",  qty_m->Render()));
            if (R::needs_price(fk))        mid_rows.push_back(labeled("price:",     price_m->Render()));
            if (R::needs_order_id(fk))     mid_rows.push_back(labeled("orderId:",   oid_m->Render()));
            if (R::needs_algo_id(fk))      mid_rows.push_back(labeled("algoId:",    aid_m->Render()));
            if (R::needs_leverage(fk))     mid_rows.push_back(labeled("leverage:",  lev_m->Render()));
            if (R::needs_margin_type(fk))  mid_rows.push_back(labeled("marginType:",mtype_m->Render()));
            if (R::needs_amount(fk))       mid_rows.push_back(labeled("amount:",    amt_m->Render()));
            if (R::needs_delta_type(fk))   mid_rows.push_back(labeled("type (1|2):",dtype_m->Render()));
            if (R::needs_countdown(fk))    mid_rows.push_back(labeled("countdown:", cntdn_m->Render()));
            if (R::needs_bool_flag(fk))    mid_rows.push_back(labeled(R::bool_flag_label(fk), bool_m->Render()));
            if (R::needs_asset(fk))        mid_rows.push_back(labeled("asset:",     asset_m->Render()));
            if (R::needs_ids_csv(fk))      mid_rows.push_back(labeled("ids (csv):", ids_m->Render()));
            if (R::needs_start_time(fk))   mid_rows.push_back(labeled("startTime:", stime_m->Render()));
            if (R::needs_end_time(fk))     mid_rows.push_back(labeled("endTime:",   etime_m->Render()));
            if (R::needs_download_id(fk))  mid_rows.push_back(labeled("downloadId:",dlid_m->Render()));
            if (R::needs_from_asset(fk))   mid_rows.push_back(labeled("from:",      from_m->Render()));
            if (R::needs_to_asset(fk))     mid_rows.push_back(labeled("to:",        to_m->Render()));
            if (R::needs_from_amount(fk))  mid_rows.push_back(labeled("amount:",    famt_m->Render()));
            if (R::needs_quote_id(fk))     mid_rows.push_back(labeled("quoteId:",   quote_m->Render()));
            if (R::needs_convert_order_id(fk)) mid_rows.push_back(labeled("orderId:", coid_m->Render()));
            if (R::needs_limit(fk))        mid_rows.push_back(labeled("limit:",     lim_m->Render()));

            mid_rows.push_back(separator());

            const entry* ent = selected_entry();
            const auto& cap = (*caps)[static_cast<std::size_t>(ent->cmd_idx)];
            const char* state_label = "";
            switch (cap->state.load()) {
                case request_capture::idle:    state_label = "idle";    break;
                case request_capture::running: state_label = "running…"; break;
                case request_capture::done:    state_label = "done";    break;
                case request_capture::failed:  state_label = "failed";  break;
            }
            mid_rows.push_back(hbox({ text("state: "), text(state_label) | bold }));
            if (blocked) {
                mid_rows.push_back(text(std::string{ "⚠ run blocked: " } + blocked)
                                   | color(Color::Yellow));
            }
            mid_rows.push_back(separator());
            mid_rows.push_back(text("info:") | dim);
            std::string info_copy;
            {
                std::lock_guard lk(cap->mtx);
                info_copy = cap->info_lines;
            }
            // Middle column is pinned at 52 chars; leave a couple for the
            // vbox border.
            mid_rows.push_back(wrap_paragraph(info_copy.empty() ? "(none)" : info_copy, 50));
        }
        mid_rows.push_back(filler());

        // Menu render wrapped in yframe so the focused row scrolls into
        // view, with a visible scroll indicator on the right edge of the
        // menu column.
        auto menu_block =
            vbox({ text("Commands") | bold, separator(),
                   cmd_menu_with_enter->Render() | vscroll_indicator | yframe | flex })
            | size(WIDTH, EQUAL, 28);

        return hbox({
                   menu_block,
                   separator(),
                   vbox(std::move(mid_rows)) | size(WIDTH, EQUAL, 52),
                   separator(),
                   rr_pane->Render() | flex,
               });
    });

    // Context-aware hints for the outer bottom bar. Focus lives in one
    // of three zones: menu, form, or the request/response pane. Before
    // any keypress no component reports focus — default to "menu" since
    // it's the top-left starting zone.
    auto hints = [cmd_menu_with_enter, form_container,
                  rr_pane]() -> std::vector<Element> {
        if (rr_pane->Focused()) {
            return {
                key_chip("PgUp/PgDn", "scroll"),
                key_chip("Tab", "sub-tabs"),
                key_chip("←", "back"),
                key_chip("q", "quit"),
            };
        }
        if (form_container->Focused()) {
            return {
                key_chip("↑↓", "next field"),
                key_chip("type", "edit"),
                key_chip("→/←", "zones"),
                key_chip("Enter", "run"),
            };
        }
        return {
            key_chip("↑↓", "select"),
            key_chip("Enter", "run/toggle"),
            key_chip("→", "form"),
            key_chip("Tab", "cycle tabs"),
            key_chip("q", "quit"),
        };
    };

    return tab_view{ component, std::move(hints) };
}

} // namespace demo_ui
