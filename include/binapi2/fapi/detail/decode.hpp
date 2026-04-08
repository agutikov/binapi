// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file decode.hpp
/// @brief Pure function: decode an HTTP response body into a typed result.

#pragma once

#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/http_client.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <glaze/glaze.hpp>

#include <type_traits>
#include <utility>

namespace binapi2::fapi::detail {

/// @brief Decode an HTTP response body into a typed result.
///
/// Checks the HTTP status code first; on non-2xx responses, attempts to parse
/// a Binance error document from the body. On success, deserializes the body
/// as JSON into @p T using glaze.
///
/// For @c types::empty_response_t, skips JSON parsing and returns an empty value.
template<typename T>
result<T>
decode_response(const transport::http_response& response)
{
    if (response.status < 200 || response.status >= 300) {
        types::binance_error_document_t error_doc{};
        glz::context ctx{};
        if (!glz::read<json_read_opts>(error_doc, response.body, ctx)) {
            return result<T>::failure({ error_code::binance, response.status, error_doc.code, error_doc.msg, response.body });
        }
        return result<T>::failure({ error_code::http_status, response.status, 0, "HTTP request failed", response.body });
    }

    if constexpr (std::is_same_v<T, types::empty_response_t>) {
        return result<T>::success({});
    } else {
        T value{};
        glz::context ctx{};
        if (auto ec = glz::read<json_read_opts>(value, response.body, ctx)) {
            return result<T>::failure(
                { error_code::json, response.status, 0, glz::format_error(ec, response.body), response.body });
        }
        return result<T>::success(std::move(value));
    }
}

} // namespace binapi2::fapi::detail
