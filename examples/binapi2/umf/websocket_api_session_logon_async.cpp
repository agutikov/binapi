#include <binapi2/umf/websocket_api/client.hpp>

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

    binapi2::umf::websocket_api::client client{ io, cfg };
    client.connect([&](binapi2::umf::result<void> connected) {
        if (!connected) {
            std::cerr << connected.err.message << '\n';
            return;
        }

        client.session_logon([&](binapi2::umf::result<binapi2::umf::types::websocket_api_response<binapi2::umf::types::session_logon_result>> result) {
            if (!result) {
                std::cerr << result.err.message << '\n';
                return;
            }

            std::cout << "status=" << result->status << '\n';
            client.close([](binapi2::umf::result<void>) {});
        });
    });

    io.run();
    return 0;
}
