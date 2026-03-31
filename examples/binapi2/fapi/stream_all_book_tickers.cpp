// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_all_book_tickers(); !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_all_book_tickers_loop([](const binapi2::fapi::types::book_ticker_stream_event& event) {
        std::cout << event.s << ' ' << event.b << ' ' << event.a << '\n';
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
