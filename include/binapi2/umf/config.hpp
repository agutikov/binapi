#pragma once

#include <cstdint>
#include <string>

namespace binapi2::umf {

struct config
{
    std::string rest_host{ "fapi.binance.com" };
    std::string rest_port{ "443" };
    std::string rest_base_path{};

    std::string websocket_api_host{ "ws-fapi.binance.com" };
    std::string websocket_api_port{ "443" };
    std::string websocket_api_target{ "/ws-fapi/v1" };

    std::string stream_host{ "fstream.binance.com" };
    std::string stream_port{ "443" };
    std::string stream_base_target{ "/ws" };

    std::string api_key{};
    std::string secret_key{};
    std::uint64_t recv_window{ 5000 };
    std::string user_agent{ "binapi2-umf/0.1.0" };
    bool testnet{ false };

    [[nodiscard]] static config testnet_config()
    {
        config cfg;
        cfg.rest_host = "testnet.binancefuture.com";
        cfg.websocket_api_host = "testnet.binancefuture.com";
        cfg.websocket_api_target = "/ws-fapi/v1";
        cfg.stream_host = "fstream.binancefuture.com";
        cfg.testnet = true;
        return cfg;
    }
};

} // namespace binapi2::umf
