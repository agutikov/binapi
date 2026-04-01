// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <type_traits>

namespace binapi2::fapi {

namespace detail {

// Detect if to_string(T) is defined for our enums.
template<class T, class = void>
struct has_fapi_to_string : std::false_type {};

template<class T>
struct has_fapi_to_string<T, std::void_t<decltype(types::to_string(std::declval<T>()))>> : std::true_type {};

template<class T>
inline constexpr bool has_fapi_to_string_v = has_fapi_to_string<T>::value;

// Convert a single field value to string for query parameters.
inline void
to_query_value(const std::string& value, std::string& out)
{
    out = value;
}

inline void
to_query_value(bool value, std::string& out)
{
    out = value ? "true" : "false";
}

inline void
to_query_value(std::uint64_t value, std::string& out)
{
    out = std::to_string(value);
}

inline void
to_query_value(int value, std::string& out)
{
    out = std::to_string(value);
}

inline void
to_query_value(double value, std::string& out)
{
    out = std::to_string(value);
}

template<class T>
    requires has_fapi_to_string_v<T>
void
to_query_value(T value, std::string& out)
{
    out = types::to_string(value);
}

// Process a single field: skip nullopt optionals, stringify the rest.
template<class T>
void
emit_field(query_map& result, std::string_view key, const T& member)
{
    if constexpr (glz::is_specialization_v<T, std::optional>) {
        if (member.has_value()) {
            std::string value;
            to_query_value(*member, value);
            result.emplace(std::string(key), std::move(value));
        }
    }
    else {
        std::string value;
        to_query_value(member, value);
        result.emplace(std::string(key), std::move(value));
    }
}

} // namespace detail

// Build a query_map from any request struct.
// Works with both glz::meta-annotated and plain reflectable structs.
// Skips std::optional fields that are nullopt.
template<class T>
[[nodiscard]] query_map
to_query_map(const T& request)
{
    query_map result;
    constexpr auto N = glz::reflect<T>::size;

    if constexpr (glz::reflectable<T>) {
        // Plain aggregate: use to_tie() for member access.
        auto tied = glz::to_tie(const_cast<T&>(request));
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (detail::emit_field(result, glz::reflect<T>::keys[I], glz::get<I>(tied)), ...);
        }(std::make_index_sequence<N>{});
    }
    else {
        // Has glz::meta: use reflect values (member pointers).
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (detail::emit_field(
                 result, glz::reflect<T>::keys[I], glz::get_member(request, glz::get<I>(glz::reflect<T>::values))),
             ...);
        }(std::make_index_sequence<N>{});
    }

    return result;
}

} // namespace binapi2::fapi
