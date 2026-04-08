// SPDX-License-Identifier: Apache-2.0

#include "common.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <cstdlib>
#include <fstream>

namespace demo {

static spdlog::level::level_enum parse_level(std::string_view s)
{
    if (s == "trace")    return spdlog::level::trace;
    if (s == "debug")    return spdlog::level::debug;
    if (s == "info")     return spdlog::level::info;
    if (s == "warn")     return spdlog::level::warn;
    if (s == "error")    return spdlog::level::err;
    if (s == "critical") return spdlog::level::critical;
    if (s == "off")      return spdlog::level::off;
    throw std::invalid_argument("unknown log level: " + std::string(s));
}

static spdlog::level::level_enum verbosity_level()
{
    switch (verbosity) {
        case 0:  return spdlog::level::info;
        case 1:  return spdlog::level::info;
        case 2:  return spdlog::level::debug;
        default: return spdlog::level::trace;
    }
}

void init_logging()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    console_sink->set_level(stdout_loglevel.empty() ? verbosity_level()
                                                    : parse_level(stdout_loglevel));

    std::vector<spdlog::sink_ptr> sinks{ console_sink };

    if (!log_file.empty()) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, /*truncate=*/true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        file_sink->set_level(file_loglevel.empty() ? verbosity_level()
                                                   : parse_level(file_loglevel));
        sinks.push_back(file_sink);
    }

    auto logger = std::make_shared<spdlog::logger>("", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace); // let sinks filter
    spdlog::set_default_logger(logger);
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

            if (!save_request_file.empty()) {
                std::ofstream f(save_request_file);
                f << e.method << ' ' << e.target << '\n';
                if (!e.body.empty()) f << e.body << '\n';
                if (!e.raw.empty())  f << e.raw << '\n';
                spdlog::info("request saved to {}", save_request_file);
            }
        } else {
            spdlog::debug("<< {} {} {} status={}", e.protocol, e.method, e.target, e.status);
            if (!e.body.empty()) spdlog::debug("<< body: {}", e.body);
            if (!e.raw.empty()) spdlog::trace("<< raw:\n{}", e.raw);

            if (!save_response_file.empty()) {
                std::ofstream f(save_response_file);
                if (!e.body.empty()) f << e.body << '\n';
                else if (!e.raw.empty()) f << e.raw << '\n';
                spdlog::info("response saved to {}", save_response_file);
            }
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
