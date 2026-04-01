#include <binapi2/fapi/client.hpp>

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
    binapi2::fapi::client client{ io, cfg };

    binapi2::fapi::types::new_order_request request{
        .symbol = "BTCUSDT",
        .side = binapi2::fapi::types::order_side::buy,
        .type = binapi2::fapi::types::order_type::limit,
        .timeInForce = binapi2::fapi::types::time_in_force::gtc,
        .quantity = "0.001",
        .price = "10000",
    };

    const auto result = client.trade.execute(request);
    if (!result) {
        std::cerr << result.err.message << '\n';
        return 1;
    }

    std::cout << "orderId=" << result->orderId << '\n';
    return 0;
}
