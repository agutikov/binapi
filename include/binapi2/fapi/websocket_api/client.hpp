// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file client.hpp
/// @brief Async WebSocket API client for Binance USD-M Futures RPC calls.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/fapi/websocket_api/endpoint_traits.hpp>

#include <boost/cobalt/task.hpp>

#include <string>
#include <type_traits>

namespace binapi2::fapi::websocket_api {

/// @brief Async WebSocket API client for Binance USD-M Futures.
///
/// All trait-enabled request types are dispatched through async_execute.
/// session_logon has custom auth and remains a named method.
class client
{
public:
    explicit client(config cfg);

    [[nodiscard]] boost::cobalt::task<result<void>> async_connect();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    /// @brief Generic execute for any request type with ws endpoint traits.
    /// Auth mode is resolved at compile time from the traits.
    template<class Request>
        requires has_ws_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<types::websocket_api_response_t<typename endpoint_traits<Request>::response_type_t>>>;

    /// @brief Session logon (custom auth flow — cannot use generic execute).
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response_t<types::session_logon_result_t>>> async_session_logon();

    // Request type aliases for discoverability.
    using book_ticker_request = types::websocket_api_book_ticker_request_t;
    using price_ticker_request = types::websocket_api_price_ticker_request_t;
    using order_place_request = types::websocket_api_order_place_request_t;
    using order_query_request = types::websocket_api_order_query_request_t;
    using order_cancel_request = types::websocket_api_order_cancel_request_t;
    using order_modify_request = types::websocket_api_order_modify_request_t;
    using position_request = types::websocket_api_position_request_t;
    using algo_order_place_request = types::websocket_api_algo_order_place_request_t;
    using algo_order_cancel_request = types::websocket_api_algo_order_cancel_request_t;
    using account_status_request = types::ws_account_status_request_t;
    using account_status_v2_request = types::ws_account_status_v2_request_t;
    using account_balance_request = types::ws_account_balance_request_t;
    using user_data_stream_start_request = types::ws_user_data_stream_start_request_t;
    using user_data_stream_ping_request = types::ws_user_data_stream_ping_request_t;
    using user_data_stream_stop_request = types::ws_user_data_stream_stop_request_t;

private:
    config cfg_;
    transport::websocket_client transport_;

    std::string next_id();
    std::uint64_t id_counter_{};

    template<typename Response, typename Params>
    boost::cobalt::task<result<types::websocket_api_response_t<Response>>>
    async_send_rpc(std::string_view method, const Params& params);

    types::websocket_api_signed_request_t make_signed_request_base();

    template<class Request>
    Request inject_auth(const Request& request);
};

} // namespace binapi2::fapi::websocket_api
