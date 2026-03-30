#include <binapi2/umf/client.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

int
main()
{
    boost::asio::io_context io;
    binapi2::umf::client client{ io, {} };

    const auto result = client.market_data.exchange_info();
    if (!result) {
        std::cerr << result.err.message << '\n';
        return 1;
    }

    std::cout << "symbols=" << result->symbols.size() << '\n';
    return 0;
}
