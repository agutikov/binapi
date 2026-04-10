// SPDX-License-Identifier: Apache-2.0

#include "common.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <cstdlib>

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
    // Shared async thread pool for all loggers — all I/O is non-blocking from coroutines.
    spdlog::init_thread_pool(8192, 1);

    // --- Main logger (console + optional file) ---
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

    auto logger = std::make_shared<spdlog::async_logger>(
        "", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(logger);

    // --- Output logger (raw stdout, no prefix — replaces std::cout) ---
    auto out_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    out_sink->set_pattern("%v");
    auto out_logger = std::make_shared<spdlog::async_logger>(
        "out", out_sink, spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    out_logger->set_level(spdlog::level::info);
    spdlog::register_logger(out_logger);

    // --- Recorder logger (raw file, no prefix — replaces ofstream) ---
    if (!record_file.empty()) {
        auto rec_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(record_file, /*truncate=*/true);
        rec_sink->set_pattern("%v");
        auto rec_logger = std::make_shared<spdlog::async_logger>(
            "rec", rec_sink, spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
        rec_logger->set_level(spdlog::level::info);
        spdlog::register_logger(rec_logger);
    }
}

void shutdown_logging()
{
    spdlog::shutdown();
}

binapi2::fapi::config make_config()
{
    auto cfg = use_testnet ? binapi2::fapi::config::testnet_config() : binapi2::fapi::config{};
    if (const char* key = std::getenv("BINANCE_API_KEY"))
        cfg.api_key = key;
    if (const char* secret = std::getenv("BINANCE_SECRET_KEY"))
        cfg.secret_key = secret;

    static constexpr std::size_t max_log_body = 512;

    cfg.logger = [](const binapi2::fapi::transport_log_entry& e) {
        auto truncate = [](const std::string& s) -> std::string {
            if (s.size() <= max_log_body) return s;
            return s.substr(0, max_log_body) + "... (" + std::to_string(s.size()) + " bytes)";
        };
        using binapi2::fapi::transport_direction;
        if (e.protocol == "CONN") {
            if (e.direction == transport_direction::sent) {
                spdlog::trace(">> {} {} ...", e.method, e.target);
            } else {
                spdlog::trace("<< {} {}{}", e.method, e.target,
                              e.body.empty() ? "" : " [" + e.body + "]");
            }
        } else if (e.direction == transport_direction::sent) {
            spdlog::debug(">> {} {} {}", e.protocol, e.method, e.target);
            if (!e.body.empty()) spdlog::debug(">> body: {}", truncate(e.body));
            if (!e.raw.empty()) spdlog::trace(">> raw:\n{}", truncate(e.raw));

            if (!save_request_file.empty()) {
                // Save request — one-shot file write, acceptable sync.
                if (auto rec = spdlog::get("out"))
                    rec->info("request saved to {}", save_request_file);
            }
        } else {
            spdlog::debug("<< {} {} {} status={}", e.protocol, e.method, e.target, e.status);
            if (!e.body.empty()) spdlog::debug("<< body: {}", truncate(e.body));
            if (!e.raw.empty()) spdlog::trace("<< raw:\n{}", truncate(e.raw));
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

std::string_view find_flag(const args_t& args, std::string_view key, std::string_view short_key)
{
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == key || (!short_key.empty() && args[i] == short_key))
            return args[i + 1];
    }
    return {};
}

} // namespace demo
