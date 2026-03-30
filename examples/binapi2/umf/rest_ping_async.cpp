#include <binapi2/umf/client.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::umf::client client{ io, {} };

    client.market_data.ping([&](binapi2::umf::result<binapi2::umf::types::empty_response> result) {
        if (!result) {
            std::cerr << result.err.message << '\n';
            return;
        }

        std::cout << "ping ok\n";
    });

    io.run();
    return 0;
}
