#include <binapi2/fapi/client.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, {} };

    const auto result = client.market_data.execute(binapi2::fapi::types::ping_request{});
    if (!result) {
        std::cerr << result.err.message << '\n';
        return 1;
    }

    std::cout << "ping ok\n";
    return 0;
}
