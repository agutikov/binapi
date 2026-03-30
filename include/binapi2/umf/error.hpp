#pragma once

#include <string>

namespace binapi2::umf {

enum class error_code {
    none = 0,
    invalid_argument,
    transport,
    http_status,
    json,
    binance,
    websocket,
    internal,
};

struct error {
    error_code code{error_code::none};
    int http_status{0};
    int binance_code{0};
    std::string message{};
    std::string payload{};
};

} // namespace binapi2::umf
