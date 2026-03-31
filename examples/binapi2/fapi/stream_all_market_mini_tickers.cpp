// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_all_market_mini_tickers(); !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_all_market_mini_tickers_loop([](const binapi2::fapi::types::all_market_mini_ticker_stream_event& events) {
        for (const auto& event : events) {
            std::cout << event.s << ' ' << event.c << ' ' << event.v << '\n';
        }
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
