// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, {} };

    if (auto connected = streams.connect_continuous_contract_kline(
            { .pair = "btcusdt", .contract_type = binapi2::fapi::types::contract_type::perpetual, .interval = binapi2::fapi::types::kline_interval::m1 });
        !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto loop = streams.read_continuous_contract_kline_loop(
        [](const binapi2::fapi::types::continuous_contract_kline_stream_event& event) {
            std::cout << event.pair << ' ' << event.contract_type << ' ' << event.kline.interval << ' ' << event.kline.close_price << '\n';
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
