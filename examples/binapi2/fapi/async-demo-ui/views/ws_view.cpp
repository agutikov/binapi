// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API tab.
//
// Flat scrollable menu (all ~16 commands in one group), Container::Vertical
// form with Maybe-wrapped inputs, virtualized request/response pane shared
// with the REST tab, and a context-aware bottom keybar.
//
// Shares `rest::form_state`, `rest::form_kind`, and `rest::cmd_ctx` with the
// REST view — a WS API command is just another `rest::rest_command` entry.
// See `ws/commands.cpp` for the registry; see `rest/commands.hpp` for the
// shared types.

#include "../app_state.hpp"
#include "../rest/commands.hpp"
#include "../util/request_capture.hpp"
#include "../worker.hpp"
#include "../ws/commands.hpp"
#include "response_pane.hpp"
#include "views.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace demo_ui {

using namespace ftxui;

namespace {

namespace R = demo_ui::rest;

} // namespace

tab_view make_ws_view(app_state& state, worker& wrk)
{
    spdlog::debug("make_ws_view: entering");

    auto cmds   = ws::ws_commands();
    auto titles = std::make_shared<std::vector<std::string>>();
    titles->reserve(cmds.size());
    for (const auto& c : cmds) titles->emplace_back(c.name);

    auto caps = std::make_shared<std::vector<std::shared_ptr<request_capture>>>();
    caps->reserve(cmds.size());
    for (std::size_t i = 0; i < cmds.size(); ++i)
        caps->emplace_back(std::make_shared<request_capture>());

    auto menu_index = std::make_shared<int>(0);
    auto form       = std::make_shared<R::form_state>();

    auto selected_cmd = [cmds, menu_index]() -> const R::rest_command* {
        auto idx = static_cast<std::size_t>(*menu_index);
        if (idx >= cmds.size()) return nullptr;
        return &cmds[idx];
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

    auto cmd_menu = Menu(titles.get(), menu_index.get());
    auto cmd_menu_with_enter = CatchEvent(cmd_menu,
        [&wrk, &state, caps, menu_index, form,
         run_blocked_reason, selected_cmd](Event e) {
            if (e != Event::Return) return false;
            if (const char* reason = run_blocked_reason()) {
                spdlog::info("ws_view: run blocked — {}", reason);
                return true;
            }
            const auto* cmd = selected_cmd();
            if (!cmd) return true;
            const auto idx = static_cast<std::size_t>(*menu_index);
            R::cmd_ctx ctx{ wrk, state, (*caps)[idx], *form };
            cmd->run(ctx);
            return true;
        });

    // ── Input widgets, Maybe-gated per form_kind ──────────────────────

    auto make_input = [](std::string* s, const char* placeholder) {
        return Input(s, placeholder);
    };

    auto sym_in   = make_input(&form->symbol,     "symbol");
    auto oid_in   = make_input(&form->order_id,   "orderId");
    auto aid_in   = make_input(&form->algo_id,    "algoId");
    auto side_in  = make_input(&form->side,       "side (BUY|SELL)");
    auto otype_in = make_input(&form->order_type, "type (LIMIT|MARKET|…)");
    auto atype_in = make_input(&form->algo_type,  "algoType (VP|TWAP)");
    auto tif_in   = make_input(&form->tif,        "tif (GTC|IOC|FOK)");
    auto qty_in   = make_input(&form->quantity,   "quantity");
    auto price_in = make_input(&form->price,      "price");

    auto maybe = [selected_form](auto widget, auto needs_fn) {
        return widget | Maybe([selected_form, needs_fn] { return needs_fn(selected_form()); });
    };

    auto sym_m   = maybe(sym_in,   R::needs_symbol);
    auto oid_m   = maybe(oid_in,   R::needs_order_id);
    auto aid_m   = maybe(aid_in,   R::needs_algo_id);
    auto side_m  = maybe(side_in,  R::needs_side);
    auto otype_m = maybe(otype_in, R::needs_order_type);
    auto atype_m = maybe(atype_in, R::needs_algo_type);
    auto tif_m   = maybe(tif_in,   R::needs_tif);
    auto qty_m   = maybe(qty_in,   R::needs_quantity);
    auto price_m = maybe(price_in, R::needs_price);

    auto form_container = Container::Vertical({
        sym_m, side_m, otype_m, atype_m, tif_m, qty_m, price_m, oid_m, aid_m,
    });

    auto rr_pane = make_request_response_pane(
        [caps, menu_index]() -> std::shared_ptr<request_capture> {
            auto idx = static_cast<std::size_t>(*menu_index);
            return (*caps)[idx];
        });

    auto root = Container::Horizontal({
        cmd_menu_with_enter,
        form_container,
        rr_pane,
    });

    cmd_menu_with_enter->TakeFocus();

    auto component = Renderer(root, [cmd_menu_with_enter, form_container, rr_pane,
                                     caps, menu_index, titles,
                                     selected_cmd, run_blocked_reason,
                                     sym_m, oid_m, aid_m, side_m, otype_m,
                                     atype_m, tif_m, qty_m, price_m] {
        const auto* cmd = selected_cmd();
        const char* blocked = run_blocked_reason();

        Elements mid_rows;
        if (!cmd) {
            mid_rows.push_back(text("(no selection)") | dim);
        } else {
            mid_rows.push_back(text(cmd->name) | bold);
            mid_rows.push_back(text(cmd->description) | dim);
            mid_rows.push_back(text(R::requires_auth(cmd->group) ? "(auth)" : "(public)") | dim);
            const auto fk = cmd->form;
            mid_rows.push_back(separator());

            auto labeled = [](const char* label, Element field) {
                return hbox({ text(label) | dim | size(WIDTH, EQUAL, 14),
                              field | border | flex });
            };

            if (R::needs_symbol(fk))     mid_rows.push_back(labeled("symbol:",   sym_m->Render()));
            if (R::needs_side(fk))       mid_rows.push_back(labeled("side:",     side_m->Render()));
            if (R::needs_order_type(fk)) mid_rows.push_back(labeled("type:",     otype_m->Render()));
            if (R::needs_algo_type(fk))  mid_rows.push_back(labeled("algoType:", atype_m->Render()));
            if (R::needs_tif(fk))        mid_rows.push_back(labeled("tif:",      tif_m->Render()));
            if (R::needs_quantity(fk))   mid_rows.push_back(labeled("quantity:", qty_m->Render()));
            if (R::needs_price(fk))      mid_rows.push_back(labeled("price:",    price_m->Render()));
            if (R::needs_order_id(fk))   mid_rows.push_back(labeled("orderId:",  oid_m->Render()));
            if (R::needs_algo_id(fk))    mid_rows.push_back(labeled("algoId:",   aid_m->Render()));

            mid_rows.push_back(separator());

            const auto idx = static_cast<std::size_t>(*menu_index);
            const auto& cap = (*caps)[idx];
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
            mid_rows.push_back(text(info_copy.empty() ? std::string{ "(none)" } : info_copy));
        }
        mid_rows.push_back(filler());

        auto menu_block =
            vbox({ text("WS API") | bold, separator(),
                   cmd_menu_with_enter->Render() | vscroll_indicator | yframe | flex })
            | size(WIDTH, EQUAL, 24);

        return hbox({
            menu_block,
            separator(),
            vbox(std::move(mid_rows)) | size(WIDTH, EQUAL, 48),
            separator(),
            rr_pane->Render() | flex,
        });
    });

    // Context-aware hints for the outer bottom bar.
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
            key_chip("Enter", "run"),
            key_chip("→", "form"),
            key_chip("Tab", "cycle tabs"),
            key_chip("q", "quit"),
        };
    };
    return tab_view{ component, std::move(hints) };
}

} // namespace demo_ui
