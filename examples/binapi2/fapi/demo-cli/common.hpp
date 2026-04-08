// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-demo-cli: demonstration client for the binapi2 fapi library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace demo {

using args_t = std::vector<std::string>;

// Verbosity: 0 = summary only, 1 = print JSON, 2 = print JSON + HTTP details.
inline int verbosity = 0;
inline bool use_testnet = true;

// Save request/response bodies to files.
inline std::string save_request_file;
inline std::string save_response_file;

// Record raw WebSocket stream frames to a JSONL file.
inline std::string record_file;

// File logging.
inline std::string log_file;
inline std::string file_loglevel;
inline std::string stdout_loglevel;

// Initialize spdlog (call once from main).
void init_logging();

// Build config from BINANCE_API_KEY / BINANCE_SECRET_KEY env vars.
binapi2::fapi::config make_config();

// Print error details via spdlog.
void print_error(const binapi2::fapi::error& err);

// Print a value as pretty JSON to stdout.
template<typename T>
void print_json(const T& value)
{
    auto json = glz::write<glz::opts{ .prettify = true }>(value);
    if (json) {
        std::cout << *json << '\n';
    }
}

// Log a value as JSON via spdlog debug.
template<typename T>
void log_json(const T& value)
{
    auto json = glz::write_json(value);
    if (json) {
        spdlog::debug("json: {}", *json);
    }
}

// Check result, print error or JSON, return exit code.
template<typename T>
int handle_result(const binapi2::fapi::result<T>& r)
{
    if (!r) {
        print_error(r.err);
        return 1;
    }
    if (verbosity >= 1) {
        print_json(*r);
    }
    return 0;
}

// Parse an enum from a string using glaze metadata (e.g. "BUY" -> order_side_t::buy).
// Enums whose glz::meta uses uppercase keys (BUY, LIMIT, GTC) accept input case-insensitively.
// Kline intervals (1m, 1h, 1M) are parsed as-is.
template<typename E>
E parse_enum(std::string_view s)
{
    // glz::read_json expects a quoted JSON string: "BUY"
    std::string quoted = "\"" + std::string(s) + "\"";
    E value{};
    if (!glz::read_json(value, quoted)) {
        return value;
    }

    // Retry with uppercase (handles case-insensitive input for BUY/SELL/LIMIT/etc.)
    std::string upper(s);
    std::ranges::transform(upper, upper.begin(), ::toupper);
    quoted = "\"" + upper + "\"";
    if (!glz::read_json(value, quoted)) {
        return value;
    }

    throw std::invalid_argument("unknown " + std::string(typeid(E).name()) + ": " + std::string(s));
}

// Find a --key (or -k short) value pair in args, return value or empty.
std::string_view find_flag(const args_t& args, std::string_view key, std::string_view short_key = {});

} // namespace demo
