// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/streams/market_streams.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_kline(
            { .symbol = "btcusdt", .interval = binapi2::fapi::types::kline_interval::m1 });
        !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_kline_loop([](const binapi2::fapi::types::kline_stream_event& event) {
        std::cout << event.s << ' ' << event.k.i << ' ' << event.k.c << ' ' << event.k.x << '\n';
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
