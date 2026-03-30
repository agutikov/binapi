#include <binapi2/umf/client.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>

int
main()
{
    boost::asio::io_context io;
    auto cfg = binapi2::umf::config::testnet_config();
    if (const char* api_key = std::getenv("BINANCE_API_KEY"))
        cfg.api_key = api_key;
    if (const char* secret_key = std::getenv("BINANCE_SECRET_KEY"))
        cfg.secret_key = secret_key;

    binapi2::umf::client client{ io, cfg };
    auto result = client.user_data_streams.start();
    if (!result) {
        std::cerr << result.err.message << '\n';
        return 1;
    }

    std::cout << result->listenKey << '\n';
    return 0;
}
