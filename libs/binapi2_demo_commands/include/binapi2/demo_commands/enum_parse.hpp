// SPDX-License-Identifier: Apache-2.0
//
// String → enum parser used by both demo binaries. Relies on the same
// `glz::meta` specialisations that the API uses for JSON (de)serialisation,
// so e.g. `parse_enum<order_side_t>("BUY")` works out of the box for every
// enum the library knows how to (de)serialise.

#pragma once

#include <glaze/glaze.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>

namespace binapi2::demo {

/// Parse an enum from a string using glaze metadata
/// (e.g. `"BUY" → order_side_t::buy`).
///
/// Enums whose `glz::meta` uses uppercase keys (BUY, LIMIT, GTC) accept input
/// case-insensitively — we retry once with the input upper-cased. Kline
/// intervals (`1m`, `1h`, `1M`) are parsed as-is on the first attempt.
template<typename E>
E parse_enum(std::string_view s)
{
    // glz::read_json expects a quoted JSON string: "BUY".
    std::string quoted = "\"" + std::string(s) + "\"";
    E value{};
    if (!glz::read_json(value, quoted)) {
        return value;
    }

    // Retry with uppercase (handles case-insensitive input for BUY/SELL/LIMIT/etc.).
    std::string upper(s);
    std::ranges::transform(upper, upper.begin(), ::toupper);
    quoted = "\"" + upper + "\"";
    if (!glz::read_json(value, quoted)) {
        return value;
    }

    throw std::invalid_argument("unknown " + std::string(typeid(E).name()) + ": " + std::string(s));
}

} // namespace binapi2::demo
