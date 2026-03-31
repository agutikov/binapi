#include <binapi2/fapi/websocket_api/client.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>

int
main()
{
    boost::asio::io_context io;
    auto cfg = binapi2::fapi::config::testnet_config();
    if (const char* api_key = std::getenv("BINANCE_API_KEY"))
        cfg.api_key = api_key;
    if (const char* secret_key = std::getenv("BINANCE_SECRET_KEY"))
        cfg.secret_key = secret_key;

    binapi2::fapi::websocket_api::client client{ io, cfg };
    client.connect([&](binapi2::fapi::result<void> connected) {
        if (!connected) {
            std::cerr << connected.err.message << '\n';
            return;
        }

        client.session_logon(
            [&](binapi2::fapi::result<binapi2::fapi::types::websocket_api_response<binapi2::fapi::types::session_logon_result>>
                    result) {
                if (!result) {
                    std::cerr << result.err.message << '\n';
                    return;
                }

                std::cout << "status=" << result->status << '\n';
                client.close([](binapi2::fapi::result<void>) {});
            });
    });

    io.run();
    return 0;
}
