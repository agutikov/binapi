#include <binapi2/umf/websocket_api/client.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    boost::asio::io_context io;
    auto cfg = binapi2::umf::config::testnet_config();
    if (const char *api_key = std::getenv("BINANCE_API_KEY")) cfg.api_key = api_key;
    if (const char *secret_key = std::getenv("BINANCE_SECRET_KEY")) cfg.secret_key = secret_key;

    binapi2::umf::websocket_api::client client{io, cfg};
    if (auto connected = client.connect(); !connected) {
        std::cerr << connected.err.message << '\n';
        return 1;
    }

    auto result = client.session_logon();
    if (!result) {
        std::cerr << result.err.message << '\n';
        return 1;
    }

    std::cout << "status=" << result->status << '\n';
    if (auto closed = client.close(); !closed) {
        std::cerr << closed.err.message << '\n';
        return 1;
    }

    return 0;
}
