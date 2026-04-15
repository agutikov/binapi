// SPDX-License-Identifier: Apache-2.0
//
// Pipeline order book demo — multi-threaded order book with 3-stage pipeline:
//   network thread → parser thread → logic thread
//
// Demonstrates: pipeline_order_book with async_run() coroutine,
// stream_consumer for cross-thread frame forwarding, and stream_parser
// for typed event parsing on a dedicated thread.

#include "cmd_pipeline_order_book.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/order_book/pipeline_order_book.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

void register_cmd_pipeline_order_book(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Order Book";

    struct opts_t { std::string symbol; int depth = 1000; };
    auto opts = std::make_shared<opts_t>();
    auto* sub = app.add_subcommand("pipeline-order-book-live",
                                   "Pipeline order book (3 threads) <symbol> [--depth D]");
    sub->group(group);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->add_option("-d,--depth", opts->depth, "Snapshot depth")->capture_default_str();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            constexpr std::size_t buffer_size = 4096;

            auto rest = co_await c.create_rest_client();
            if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }

            binapi2::fapi::order_book::pipeline_order_book book(
                c.configuration(), (*rest)->market_data, buffer_size);
            // Note: the pipeline order book runs across multiple threads and needs
            // a cross-thread buffer, so it can't reuse the demo CLI's async
            // (single-executor) record_buffer. Recording is not wired up here.

            sink.on_info("starting pipeline order book for " + opts->symbol
                         + " depth=" + std::to_string(opts->depth) + " (3 threads)");

            book.set_snapshot_callback([opts](const binapi2::fapi::order_book::order_book_snapshot& snap) {
                constexpr int display_levels = 10;
                spdlog::trace("pipeline order book update: id={} bids={} asks={}",
                              snap.last_update_id, snap.bids.size(), snap.asks.size());

                out("\033[2J\033[H");
                out("=== {} Pipeline Order Book (update {}) ===\n", opts->symbol, snap.last_update_id);

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
            if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
            sink.on_done(0);
            co_return 0;
        };
    });
}

} // namespace demo
