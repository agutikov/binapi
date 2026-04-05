// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file timestamp.hpp
/// @brief Millisecond-precision UTC timestamp for the Binance wire format.
///
/// Binance represents all timestamps as integer milliseconds since the Unix
/// epoch (1970-01-01T00:00:00Z).  This thin wrapper preserves that
/// representation while giving the type system enough information to
/// distinguish timestamps from other integer fields (order IDs, counts, etc.).

#pragma once

#include <cstdint>
#include <string>

namespace binapi2::fapi::types {

/// @brief Milliseconds since the Unix epoch.
///
/// Wraps a plain `std::uint64_t` so that request/response struct fields
/// are self-documenting and cannot be accidentally mixed with non-timestamp
/// integers.
struct timestamp_ms
{
    std::uint64_t value{};

    constexpr timestamp_ms() = default;
    constexpr explicit timestamp_ms(std::uint64_t v) : value(v) {}

    constexpr auto operator<=>(const timestamp_ms&) const = default;
    constexpr bool operator==(const timestamp_ms&) const = default;

    /// @brief Convert to the query-string representation expected by Binance.
    [[nodiscard]] std::string to_string() const { return std::to_string(value); }
};

} // namespace binapi2::fapi::types

// -- Glaze JSON serialization ------------------------------------------------
// On the wire a timestamp is a plain JSON integer.

#include <glaze/glaze.hpp>

namespace glz {

template<>
struct from<JSON, binapi2::fapi::types::timestamp_ms>
{
    template<auto Opts>
    static void op(binapi2::fapi::types::timestamp_ms& ts, is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::uint64_t v{};
        parse<JSON>::op<Opts>(v, ctx, it, end);
        ts.value = v;
    }
};

template<>
struct to<JSON, binapi2::fapi::types::timestamp_ms>
{
    template<auto Opts>
    static void op(const binapi2::fapi::types::timestamp_ms& ts, is_context auto&& ctx, auto&& b, auto&& ix) noexcept
    {
        serialize<JSON>::op<Opts>(ts.value, ctx, b, ix);
    }
};

} // namespace glz
