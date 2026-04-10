// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file variant_parse.hpp
/// @brief Generic discriminator-based JSON variant parser.
///
/// Parses a JSON object into a std::variant by:
///   1. Extracting a discriminator field value (single fast scan).
///   2. Looking up the value in a compile-time mapping.
///   3. Parsing once into the matched type via glaze.
///
/// This avoids the try-parse-rewind pattern where each variant alternative
/// is attempted sequentially until one succeeds.
///
/// Usage:
///
///   constexpr variant_entry<my_variant_t> mapping[] = {
///       make_entry<event_a_t>("EVENT_A"),
///       make_entry<event_b_t>("EVENT_B"),
///   };
///
///   auto result = parse_variant("e", json, mapping);

#pragma once

#include <binapi2/fapi/detail/json_opts.hpp>

#include <glaze/glaze.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace binapi2::fapi::detail {

// ---------------------------------------------------------------------------
// Discriminator field extraction
// ---------------------------------------------------------------------------

/// @brief Extract the string value of a JSON field without full parse.
///
/// Scans for "key":"value" or "key": "value" at the top level of the JSON
/// object. Returns empty string_view if the key is not found or the value
/// is not a string.
///
/// This is intentionally simple — it handles the common case of flat event
/// objects with a string discriminator field. It does NOT handle:
///   - keys inside nested objects (only top-level depth is scanned)
///   - escaped quotes inside the discriminator value
///   - non-string discriminator values
inline std::string_view extract_string_field(std::string_view json, std::string_view key)
{
    // Build the search pattern: "key"
    // We look for the exact key with surrounding quotes.
    char needle[128];
    if (key.size() + 3 > sizeof(needle)) return {};
    needle[0] = '"';
    std::copy(key.begin(), key.end(), needle + 1);
    needle[key.size() + 1] = '"';
    needle[key.size() + 2] = '\0';
    const std::string_view needle_sv(needle, key.size() + 2);

    // Find the key at top level (brace depth 1)
    int depth = 0;
    std::size_t i = 0;
    while (i < json.size()) {
        char c = json[i];
        if (c == '{' || c == '[') {
            ++depth;
            ++i;
        } else if (c == '}' || c == ']') {
            --depth;
            ++i;
        } else if (c == '"' && depth == 1) {
            // Check if this is our key
            if (json.substr(i, needle_sv.size()) == needle_sv) {
                // Found the key — skip past it and the colon
                i += needle_sv.size();
                while (i < json.size() && (json[i] == ' ' || json[i] == ':'))
                    ++i;
                // Now extract the string value
                if (i < json.size() && json[i] == '"') {
                    ++i; // skip opening quote
                    std::size_t start = i;
                    while (i < json.size() && json[i] != '"')
                        ++i;
                    return json.substr(start, i - start);
                }
                return {};
            }
            // Skip past this string
            ++i; // skip opening quote
            while (i < json.size() && json[i] != '"') {
                if (json[i] == '\\') ++i; // skip escaped char
                ++i;
            }
            if (i < json.size()) ++i; // skip closing quote
        } else {
            ++i;
        }
    }
    return {};
}

// ---------------------------------------------------------------------------
// Variant dispatch table
// ---------------------------------------------------------------------------

/// @brief A mapping entry: discriminator value → parse function.
///
/// @tparam Variant The std::variant type to construct.
template<typename Variant>
struct variant_entry
{
    std::string_view discriminator_value;

    /// Attempt to parse the JSON into the specific type and wrap in Variant.
    /// Returns true on success, false on parse failure.
    bool (*try_parse)(const std::string& json, Variant& out);
};

/// @brief Create a variant_entry for type T with the given discriminator value.
///
/// @tparam T      The concrete type (must be an alternative of Variant).
/// @tparam Variant Deduced from context.
template<typename T, typename Variant>
constexpr variant_entry<Variant> make_entry(std::string_view discriminator_value)
{
    return {
        discriminator_value,
        [](const std::string& json, Variant& out) -> bool {
            T value{};
            glz::context ctx{};
            if (glz::read<json_read_opts>(value, json, ctx)) return false;
            out = std::move(value);
            return true;
        }
    };
}

// ---------------------------------------------------------------------------
// Parse function
// ---------------------------------------------------------------------------

/// @brief Parse JSON into a variant using discriminator-based dispatch.
///
/// Extracts the discriminator field value, looks it up in the mapping table,
/// and parses the JSON into the matched type. Exactly one glz::read call
/// is made (the matched type), regardless of how many variant alternatives
/// exist.
///
/// @param discriminator_key  The JSON field name used to discriminate (e.g. "e").
/// @param json               The raw JSON string.
/// @param mapping            Array of variant_entry mapping discriminator values to types.
/// @return                   The parsed variant, or std::nullopt on failure.
template<typename Variant, std::size_t N>
std::optional<Variant> parse_variant(std::string_view discriminator_key,
                                     const std::string& json,
                                     const variant_entry<Variant> (&mapping)[N])
{
    auto disc_value = extract_string_field(json, discriminator_key);
    if (disc_value.empty()) return std::nullopt;

    for (const auto& entry : mapping) {
        if (entry.discriminator_value == disc_value) {
            Variant out;
            if (entry.try_parse(json, out))
                return out;
            return std::nullopt;
        }
    }
    return std::nullopt;
}

} // namespace binapi2::fapi::detail
