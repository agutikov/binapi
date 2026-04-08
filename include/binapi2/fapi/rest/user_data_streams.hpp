// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief User data stream service for managing WebSocket listen keys.

#pragma once

#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

/// @brief Service for managing user data stream listen keys.
///
/// A listen key is required to open a WebSocket user data stream that delivers
/// real-time account updates (order fills, balance changes, margin calls).
///
/// Lifecycle:
///   1. Call async_start() to obtain a new listen key (valid for 60 minutes).
///   2. Call async_keepalive() periodically (recommended every 30 minutes) to extend validity.
///   3. Call async_close() when the stream is no longer needed.
class user_data_stream_service
{
public:
    explicit user_data_stream_service(pipeline& p) noexcept;

    /// @brief Create a new listen key for opening a user data WebSocket stream.
    [[nodiscard]] boost::cobalt::task<result<types::listen_key_response_t>> async_start();

    /// @brief Extend the validity of the current listen key by another 60 minutes.
    [[nodiscard]] boost::cobalt::task<result<types::listen_key_response_t>> async_keepalive();

    /// @brief Invalidate the current listen key and close the associated data stream.
    [[nodiscard]] boost::cobalt::task<result<types::empty_response_t>> async_close();

private:
    pipeline& pipeline_;
};

} // namespace binapi2::fapi::rest
