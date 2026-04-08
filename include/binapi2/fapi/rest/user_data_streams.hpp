// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief User data stream service for managing WebSocket listen keys.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

/// @brief Service for managing user data stream listen keys.
///
/// A listen key is required to open a WebSocket user data stream that delivers
/// real-time account updates (order fills, balance changes, margin calls).
///
/// Lifecycle:
///   1. Call start() to obtain a new listen key (valid for 60 minutes).
///   2. Call keepalive() periodically (recommended every 30 minutes) to extend validity.
///   3. Call close() when the stream is no longer needed.
///
/// Unlike other services, this class does not inherit from rest::service because
/// listen key endpoints use user_stream security (API key header only, no HMAC signing)
/// and have no request/response types with endpoint_traits mappings.
class user_data_stream_service
{
public:
    explicit user_data_stream_service(client& owner) noexcept;

    /// @brief Create a new listen key for opening a user data WebSocket stream.
    /// @return The listen key string on success, or an error.
    [[nodiscard]] result<types::listen_key_response_t> start();
    /// @brief Async variant of start.
    [[nodiscard]] boost::cobalt::task<result<types::listen_key_response_t>> async_start();

    /// @brief Extend the validity of the current listen key by another 60 minutes.
    /// @return The listen key on success, or an error.
    [[nodiscard]] result<types::listen_key_response_t> keepalive();
    /// @brief Async variant of keepalive.
    [[nodiscard]] boost::cobalt::task<result<types::listen_key_response_t>> async_keepalive();

    /// @brief Invalidate the current listen key and close the associated data stream.
    /// @return Empty response on success, or an error.
    [[nodiscard]] result<types::empty_response_t> close();
    /// @brief Async variant of close.
    [[nodiscard]] boost::cobalt::task<result<types::empty_response_t>> async_close();

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
