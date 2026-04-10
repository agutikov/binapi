// SPDX-License-Identifier: Apache-2.0
//
// Local order book demo — real-time order book synchronized from the diff depth
// stream with a REST snapshot, displayed in the terminal with ANSI escape codes.
// Demonstrates: local_order_book with async_run() coroutine.

#include "cmd_order_book.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/streams/local_order_book.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_order_book_live(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: order-book-live <symbol> [depth]"); co_return 1; }

    auto streams = c.create_market_streams();
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    const std::string symbol = args[0];
    const int depth = (args.size() > 1) ? std::stoi(args[1]) : 1000;
    constexpr int display_levels = 10;

    binapi2::fapi::streams::local_order_book book(*streams, (*rest)->rest_pipeline());

    spdlog::info("starting local order book for {} depth={}", symbol, depth);

    book.set_snapshot_callback([&](const binapi2::fapi::streams::order_book_snapshot& snap) {
        spdlog::trace("order book update: id={} bids={} asks={}",
                      snap.last_update_id, snap.bids.size(), snap.asks.size());

        out("\033[2J\033[H");
        out("=== {} Order Book (update {}) ===\n", symbol, snap.last_update_id);

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

    auto r = co_await book.async_run(types::symbol_t{symbol}, depth);
    if (!r) { print_error(r.err); co_return 1; }
    co_return 0;
}

} // namespace demo
