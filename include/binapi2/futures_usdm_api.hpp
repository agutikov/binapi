// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Top-level USD-M Futures API — configuration holder and async factory
///        for REST clients, WebSocket API clients, and stream clients.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/rest/client.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/streams/user_streams.hpp>
#include <binapi2/fapi/websocket_api/client.hpp>

#include <boost/cobalt/task.hpp>

#include <memory>

namespace binapi2 {

/// @brief USD-M Futures API — configuration holder and async factory.
///
/// Creates independent REST clients, WebSocket API clients, and stream clients.
/// REST and WebSocket API clients are connected before being returned.
///
///   futures_usdm_api api(cfg);
///   auto rest = co_await api.create_rest_client();
///   auto r = co_await rest->market_data.async_execute(ping_request_t{});
///
///   auto ws = co_await api.create_ws_api_client();
class futures_usdm_api
{
public:
    explicit futures_usdm_api(fapi::config cfg);

    futures_usdm_api(const futures_usdm_api&) = delete;
    futures_usdm_api& operator=(const futures_usdm_api&) = delete;

    [[nodiscard]] fapi::config& configuration() noexcept;
    [[nodiscard]] const fapi::config& configuration() const noexcept;

    /// @brief Create a REST client and establish the HTTP connection.
    [[nodiscard]] boost::cobalt::task<fapi::result<std::unique_ptr<fapi::rest::client>>>
    create_rest_client();

    /// @brief Create a WebSocket API client and establish the connection.
    [[nodiscard]] boost::cobalt::task<fapi::result<std::unique_ptr<fapi::websocket_api::client>>>
    create_ws_api_client();

    /// @brief Create a market data stream client (connect via subscribe).
    [[nodiscard]] std::unique_ptr<fapi::streams::market_streams> create_market_streams();

    /// @brief Create a user data stream client (connect via subscribe).
    [[nodiscard]] std::unique_ptr<fapi::streams::user_streams> create_user_streams();

private:
    fapi::config cfg_;
};

} // namespace binapi2
