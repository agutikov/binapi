// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_liquidation_order({ .symbol = "btcusdt" }); !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_liquidation_order_loop([](const binapi2::fapi::types::liquidation_order_stream_event& event) {
        std::cout << event.o.s << ' ' << event.o.S << ' ' << event.o.ap << '\n';
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
