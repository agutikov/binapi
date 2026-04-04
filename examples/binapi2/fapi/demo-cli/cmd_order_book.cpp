// SPDX-License-Identifier: Apache-2.0
//
// Local order book demo — real-time order book synchronized from the diff depth
// stream with a REST snapshot, displayed in the terminal with ANSI escape codes.
// Demonstrates: local_order_book class, snapshot_callback, thread-safe access.

#include "cmd_order_book.hpp"

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/streams/local_order_book.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <iomanip>
#include <iostream>

namespace demo {

int cmd_order_book_live(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: order-book-live <symbol> [depth]\n"; return 1; }

    const std::string symbol = args[0];
    const int depth = (args.size() > 1) ? std::stoi(args[1]) : 1000;
    constexpr int display_levels = 10;

    boost::asio::io_context io;
    auto cfg = make_config();

    binapi2::fapi::client rest{ io, cfg };
    binapi2::fapi::streams::market_streams streams{ io, cfg };
    binapi2::fapi::streams::local_order_book book{ streams, rest };

    spdlog::info("starting local order book for {} depth={}", symbol, depth);

    book.set_snapshot_callback([&](const binapi2::fapi::streams::order_book_snapshot& snap) {
        spdlog::trace("order book update: id={} bids={} asks={}",
                      snap.last_update_id, snap.bids.size(), snap.asks.size());

        std::cout << "\033[2J\033[H";
        std::cout << "=== " << symbol << " Order Book (update " << snap.last_update_id << ") ===\n\n";

        std::cout << "  ASKS (best " << display_levels << ")\n";
        std::vector<std::pair<binapi2::fapi::types::decimal, binapi2::fapi::types::decimal>> top_asks;
        {
            auto it = snap.asks.begin();
            for (int i = 0; i < display_levels && it != snap.asks.end(); ++i, ++it)
                top_asks.emplace_back(it->first, it->second);
        }
        for (auto it = top_asks.rbegin(); it != top_asks.rend(); ++it)
            std::cout << "    " << std::setw(16) << it->first << "  " << it->second << '\n';

        std::cout << "  --------------------\n";

        std::cout << "  BIDS (best " << display_levels << ")\n";
        {
            auto it = snap.bids.begin();
            for (int i = 0; i < display_levels && it != snap.bids.end(); ++i, ++it)
                std::cout << "    " << std::setw(16) << it->first << "  " << it->second << '\n';
        }
        std::cout << std::flush;
    });

    auto r = book.start(symbol, depth);
    if (!r) { print_error(r.err); return 1; }
    return 0;
}

} // namespace demo
