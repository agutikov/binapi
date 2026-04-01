#include <binapi2/fapi/client.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, {} };

    client.market_data.async_execute(binapi2::fapi::types::ping_request{}, [&](binapi2::fapi::result<binapi2::fapi::types::empty_response> result) {
        if (!result) {
            std::cerr << result.err.message << '\n';
            return;
        }

        std::cout << "ping ok\n";
    });

    io.run();
    return 0;
}
