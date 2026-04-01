// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file query.hpp
/// @brief Compile-time reflection of request structs into `query_map` via the
///        glaze library, so that typed request objects can be transparently
///        serialised into Binance query parameters.

#pragma once

#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/types/decimal.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <type_traits>

namespace binapi2::fapi {

namespace detail {

/// @brief SFINAE trait that detects whether `types::to_string(T)` is a valid
///        expression.
///
/// This is used to route fapi enum values through their custom stringifier
/// rather than a generic `std::to_string` call.
template<class T, class = void>
struct has_fapi_to_string : std::false_type {};

template<class T>
struct has_fapi_to_string<T, std::void_t<decltype(types::to_string(std::declval<T>()))>> : std::true_type {};

template<class T>
inline constexpr bool has_fapi_to_string_v = has_fapi_to_string<T>::value;

/// @brief Overload set that converts a single typed field value into its string
///        representation suitable for a URL query parameter.
/// @{
inline void
to_query_value(const std::string& value, std::string& out)
{
    out = value;
}

inline void
to_query_value(const types::decimal& value, std::string& out)
{
    out = value.to_string();
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

/// @brief Overload for fapi enum types that have a `types::to_string` mapping.
template<class T>
    requires has_fapi_to_string_v<T>
void
to_query_value(T value, std::string& out)
{
    out = types::to_string(value);
}
/// @}

/// @brief Emit a single struct field into @p result, using its glaze-reflected
///        key name.
///
/// `std::optional` fields that are `nullopt` are silently skipped, allowing
/// request structs to omit optional Binance parameters.
/// @tparam T       The field type (may be `std::optional<U>`).
/// @param  result  Destination query map.
/// @param  key     The query-parameter name (from glaze reflection).
/// @param  member  The field value.
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

/// @brief Convert a request struct into a `query_map` by reflecting over its
///        fields at compile time.
///
/// Field names are obtained via `glz::reflect<T>::keys` (the glaze reflection
/// mechanism).  Two struct styles are supported:
///
///  - **Plain aggregates** (`glz::reflectable<T>`) -- glaze deduces members
///    automatically via `glz::to_tie()`.
///  - **Structs with explicit `glz::meta`** -- member pointers are listed in
///    `glz::reflect<T>::values`.
///
/// `std::optional` fields that are `nullopt` are omitted from the map, so
/// callers can leave optional Binance parameters unset.
///
/// @tparam T  A request struct with glaze reflection support.
/// @param  request  The request object to serialise.
/// @return A `query_map` ready for signing and query-string building.
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
