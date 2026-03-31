// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_partial_book_depth({ .symbol = "btcusdt", .levels = 5 }); !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_partial_book_depth_loop([](const binapi2::fapi::types::depth_stream_event& event) {
        std::cout << event.s << ' ' << event.u << ' ' << event.b.size() << ' ' << event.a.size() << '\n';
        return false;
    });
    if (!loop) {
        std::cerr << loop.err.message << '\n';
        return 1;
    }

    if (auto closed = streams.close(); !closed) {
        std::cerr << closed.err.message << '\n';
        return 1;
    }

    return 0;
}
