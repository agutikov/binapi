// SPDX-License-Identifier: Apache-2.0
//
// Local order book demo — real-time order book synchronized from the diff depth
// stream with a REST snapshot, displayed in the terminal with ANSI escape codes.
// Demonstrates: local_order_book with async_run() coroutine.

#include "cmd_order_book.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/order_book/local_order_book.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

void register_cmd_order_book(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Order Book";

    struct opts_t { std::string symbol; int depth = 1000; };
    auto opts = std::make_shared<opts_t>();
    auto* sub = app.add_subcommand("order-book-live", "Live order book <symbol> [--depth D]");
    sub->group(group);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->add_option("-d,--depth", opts->depth, "Snapshot depth")->capture_default_str();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
            auto streams = c.create_market_stream();
            if (record_buffer) streams->connection().attach_buffer(*record_buffer);
            auto rest = co_await c.create_rest_client();
            if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }

            binapi2::fapi::order_book::local_order_book book(*streams, (*rest)->market_data);

            spdlog::info("starting local order book for {} depth={}", opts->symbol, opts->depth);

            book.set_snapshot_callback([opts](const binapi2::fapi::order_book::order_book_snapshot& snap) {
                constexpr int display_levels = 10;
                spdlog::trace("order book update: id={} bids={} asks={}",
                              snap.last_update_id, snap.bids.size(), snap.asks.size());

                out("\033[2J\033[H");
                out("=== {} Order Book (update {}) ===\n", opts->symbol, snap.last_update_id);

                out("  ASKS (best {})", display_levels);
                std::vector<std::pair<types::decimal_t, types::decimal_t>> top_asks;
                {
                    auto it = snap.asks.begin();
                    for (int i = 0; i < display_levels && it != snap.asks.end(); ++i, ++it)
                        top_asks.emplace_back(it->first, it->second);
                }
                for (auto it = top_asks.rbegin(); it != top_asks.rend(); ++it)
                    out("    {:>16}  {}", it->first, it->second);

                out("  --------------------");

                out("  BIDS (best {})", display_levels);
                {
                    auto it = snap.bids.begin();
                    for (int i = 0; i < display_levels && it != snap.bids.end(); ++i, ++it)
                        out("    {:>16}  {}", it->first, it->second);
                }
            });

            auto r = co_await book.async_run(types::symbol_t{ opts->symbol }, opts->depth);
            if (!r) { print_error(r.err); co_return 1; }
            co_return 0;
        };
    });
}

} // namespace demo
