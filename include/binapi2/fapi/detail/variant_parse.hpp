// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file variant_parse.hpp
/// @brief Enum-based discriminator dispatch for JSON variant parsing.
///
/// Parses a JSON object into a std::variant by:
///   1. Extracting the "e" discriminator field string from JSON.
///   2. Converting to an enum via glaze enum metadata.
///   3. Looking up the enum in a constexpr dispatch table.
///   4. Parsing once into the matched type via glaze.

#pragma once

#include <binapi2/fapi/detail/json_opts.hpp>

#include <glaze/glaze.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace binapi2::fapi::detail {

// ---------------------------------------------------------------------------
// Discriminator field extraction
// ---------------------------------------------------------------------------

/// @brief Extract the string value of a JSON field without full parse.
///
/// Scans for "key":"value" at the top level of the JSON object.
/// Returns empty string_view if not found.
inline std::string_view extract_string_field(std::string_view json, std::string_view key)
{
    char needle[128];
    if (key.size() + 3 > sizeof(needle)) return {};
    needle[0] = '"';
    std::copy(key.begin(), key.end(), needle + 1);
    needle[key.size() + 1] = '"';
    needle[key.size() + 2] = '\0';
    const std::string_view needle_sv(needle, key.size() + 2);

    int depth = 0;
    std::size_t i = 0;
    while (i < json.size()) {
        char c = json[i];
        if (c == '{' || c == '[') { ++depth; ++i; }
        else if (c == '}' || c == ']') { --depth; ++i; }
        else if (c == '"' && depth == 1) {
            if (json.substr(i, needle_sv.size()) == needle_sv) {
                i += needle_sv.size();
                while (i < json.size() && (json[i] == ' ' || json[i] == ':')) ++i;
                if (i < json.size() && json[i] == '"') {
                    ++i;
                    std::size_t start = i;
                    while (i < json.size() && json[i] != '"') ++i;
                    return json.substr(start, i - start);
                }
                return {};
            }
            ++i;
            while (i < json.size() && json[i] != '"') { if (json[i] == '\\') ++i; ++i; }
            if (i < json.size()) ++i;
        } else { ++i; }
    }
    return {};
}

// ---------------------------------------------------------------------------
// Enum-based variant dispatch
// ---------------------------------------------------------------------------

/// @brief A dispatch table entry: enum value → parse function.
///
/// @tparam Enum    The discriminator enum type (e.g. market_event_type_t).
/// @tparam Variant The std::variant type to construct.
template<typename Enum, typename Variant>
struct variant_entry
{
    Enum discriminator;
    bool (*try_parse)(std::string_view json, Variant& out);
};

/// @brief Create a variant_entry for type T with the given enum discriminator.
template<typename T, typename Enum, typename Variant>
constexpr variant_entry<Enum, Variant> make_entry(Enum discriminator)
{
    return {
        discriminator,
        [](std::string_view json, Variant& out) -> bool {
            T value{};
            glz::context ctx{};
            if (glz::read<json_read_opts>(value, json, ctx)) return false;
            out = std::move(value);
            return true;
        }
    };
}

/// @brief Convert a discriminator field string to an enum value.
///
/// Uses glaze enum metadata for the conversion.
template<typename Enum>
std::optional<Enum> parse_enum(std::string_view str)
{
    Enum value{};
    const std::string quoted = "\"" + std::string(str) + "\"";
    glz::context ctx{};
    if (glz::read<glz::opts{}>(value, quoted, ctx)) return std::nullopt;
    return value;
}

/// @brief Parse JSON into a variant using enum-based discriminator dispatch.
///
/// Extracts the "e" field, converts to enum, looks up in the dispatch table,
/// and parses into the matched type.
template<typename Enum, typename Variant, std::size_t N>
std::optional<Variant> parse_variant(std::string_view discriminator_key,
                                     std::string_view json,
                                     const variant_entry<Enum, Variant> (&mapping)[N])
{
    auto disc_str = extract_string_field(json, discriminator_key);
    if (disc_str.empty()) return std::nullopt;

    auto disc_enum = parse_enum<Enum>(disc_str);
    if (!disc_enum) return std::nullopt;

    for (const auto& entry : mapping) {
        if (entry.discriminator == *disc_enum) {
            Variant out;
            if (entry.try_parse(json, out))
                return out;
            return std::nullopt;
        }
    }
    return std::nullopt;
}

/// @brief Look up a pre-resolved enum value in the dispatch table and parse.
///
/// Used when the discriminator has already been extracted and converted
/// (e.g. from a combined stream wrapper where the "e" field is in the "data"
/// sub-object).
template<typename Enum, typename Variant, std::size_t N>
std::optional<Variant> dispatch_variant(Enum disc_value,
                                        std::string_view json,
                                        const variant_entry<Enum, Variant> (&mapping)[N])
{
    for (const auto& entry : mapping) {
        if (entry.discriminator == disc_value) {
            Variant out;
            if (entry.try_parse(json, out))
                return out;
            return std::nullopt;
        }
    }
    return std::nullopt;
}

} // namespace binapi2::fapi::detail
