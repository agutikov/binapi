#include <binapi2/fapi/streams/market_streams.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_mark_price({ .symbol = "btcusdt", .every_1s = true }); !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_mark_price_loop([](const binapi2::fapi::types::mark_price_stream_event& event) {
        std::cout << event.symbol << ' ' << event.mark_price << ' ' << event.funding_rate << '\n';
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
