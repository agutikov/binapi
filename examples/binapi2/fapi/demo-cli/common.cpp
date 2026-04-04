// SPDX-License-Identifier: Apache-2.0

#include "common.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>

namespace demo {

void init_logging()
{
    //        spdlog level   print_json   transport log
    // -v     info           yes          no
    // -vv    debug          yes          summary (method, target, status, body)
    // -vvv   trace          yes          full HTTP with headers
    switch (verbosity) {
        case 0:  spdlog::set_level(spdlog::level::info);  break;
        case 1:  spdlog::set_level(spdlog::level::info);  break;
        case 2:  spdlog::set_level(spdlog::level::debug); break;
        default: spdlog::set_level(spdlog::level::trace); break;
    }
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
}

binapi2::fapi::config make_config()
{
    auto cfg = use_testnet ? binapi2::fapi::config::testnet_config() : binapi2::fapi::config{};
    if (const char* key = std::getenv("BINANCE_API_KEY"))
        cfg.api_key = key;
    if (const char* secret = std::getenv("BINANCE_SECRET_KEY"))
        cfg.secret_key = secret;

    cfg.logger = [](const binapi2::fapi::transport_log_entry& e) {
        using binapi2::fapi::transport_direction;
        if (e.protocol == "CONN") {
            // Connection lifecycle — trace only (-vvv).
            if (e.direction == transport_direction::sent) {
                spdlog::trace(">> {} {} ...", e.method, e.target);
            } else {
                spdlog::trace("<< {} {}{}", e.method, e.target,
                              e.body.empty() ? "" : " [" + e.body + "]");
            }
        } else if (e.direction == transport_direction::sent) {
            spdlog::debug(">> {} {} {}", e.protocol, e.method, e.target);
            if (!e.body.empty()) spdlog::debug(">> body: {}", e.body);
            if (!e.raw.empty()) spdlog::trace(">> raw:\n{}", e.raw);
        } else {
            spdlog::debug("<< {} {} {} status={}", e.protocol, e.method, e.target, e.status);
            if (!e.body.empty()) spdlog::debug("<< body: {}", e.body);
            if (!e.raw.empty()) spdlog::trace("<< raw:\n{}", e.raw);
        }
    };

    spdlog::info("config: {}:{} testnet={} api_key={}",
                 cfg.rest_host, cfg.rest_port, cfg.testnet,
                 cfg.api_key.empty() ? "(none)" : "***");
    spdlog::debug("config: rest={} ws_api={} stream={}",
                  cfg.rest_host, cfg.websocket_api_host, cfg.stream_host);

    return cfg;
}

void print_error(const binapi2::fapi::error& err)
{
    spdlog::error("{}", err.message);
    if (err.http_status)
        spdlog::error("  http_status: {}", err.http_status);
    if (err.binance_code)
        spdlog::error("  binance_code: {}", err.binance_code);
    if (!err.payload.empty())
        spdlog::debug("  payload: {}", err.payload);
}

std::string_view find_flag(const args_t& args, std::string_view key)
{
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == key)
            return args[i + 1];
    }
    return {};
}

} // namespace demo
