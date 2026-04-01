#include <binapi2/fapi/client.hpp>

#include <boost/cobalt/main.hpp>

#include <iostream>

boost::cobalt::main co_main(int, char*[])
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, {} };

    auto result = co_await client.market_data.async_execute(binapi2::fapi::types::ping_request{});
    if (!result) {
        std::cerr << result.err.message << '\n';
        co_return 1;
    }

    std::cout << "ping ok\n";
    co_return 0;
}
