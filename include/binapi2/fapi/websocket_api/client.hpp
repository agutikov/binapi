// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file client.hpp
/// @brief High-level WebSocket API client for Binance USD-M Futures RPC calls.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/fapi/websocket_api/endpoint_traits.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/cobalt/task.hpp>

#include <string>
#include <type_traits>

namespace binapi2::fapi::websocket_api {

/// @brief WebSocket API client for sending JSON-RPC style requests to Binance.
///
/// Provides both generic (@ref execute / @ref async_execute) and named methods
/// for interacting with the Binance Futures WebSocket API. Request types that
/// satisfy @ref has_ws_endpoint_traits can be dispatched through the generic
/// path; the trait maps each request to its RPC method name and response type.
///
/// Authentication (API key + signature) is automatically injected into signed
/// requests via @ref inject_auth. Each call is assigned a unique ID for
/// request-response correlation.
class client
{
public:
    /// @brief Construct a WebSocket API client.
    /// @param io_context Boost.Asio I/O context for async operations.
    /// @param cfg        Configuration with endpoint URL and API credentials.
    client(boost::asio::io_context& io_context, config cfg);

    // -- Connection management --

    /// @brief Synchronously connect to the WebSocket API endpoint.
    /// @return A result indicating success or connection error.
    [[nodiscard]] result<void> connect();

    /// @brief Asynchronously connect to the WebSocket API endpoint.
    /// @return A cobalt task yielding a result indicating success or error.
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect();

    /// @brief Synchronously close the WebSocket API connection.
    /// @return A result indicating success or close error.
    [[nodiscard]] result<void> close();

    /// @brief Asynchronously close the WebSocket API connection.
    /// @return A cobalt task yielding a result indicating success or error.
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    // -- Generic execute for request types with ws endpoint traits --

    /// @brief Synchronously execute a WebSocket API request.
    ///
    /// Uses @ref endpoint_traits to resolve the RPC method name and response
    /// type from the @p Request type. Authentication is injected automatically
    /// for signed endpoints.
    ///
    /// @tparam Request A request type satisfying @ref has_ws_endpoint_traits.
    /// @param request The request parameters.
    /// @return A result containing the typed WebSocket API response.
    template<class Request>
        requires has_ws_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request)
        -> result<types::websocket_api_response<typename endpoint_traits<Request>::response_type_t>>;

    /// @brief Asynchronously execute a WebSocket API request.
    ///
    /// Coroutine counterpart of @ref execute.
    ///
    /// @tparam Request A request type satisfying @ref has_ws_endpoint_traits.
    /// @param request The request parameters.
    /// @return A cobalt task yielding a result with the typed response.
    template<class Request>
        requires has_ws_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<types::websocket_api_response<typename endpoint_traits<Request>::response_type_t>>>;

    // -- Session logon (unique auth flow) --

    /// @brief Synchronously perform session logon with API key authentication.
    /// @return A result containing the session logon response.
    [[nodiscard]] result<types::websocket_api_response<types::session_logon_result>> session_logon();

    /// @brief Asynchronously perform session logon.
    /// @return A cobalt task yielding the session logon response.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::session_logon_result>>> async_session_logon();

    // -- Parameterless signed endpoints --

    /// @brief Query account information (v1).
    /// @return A result containing the account information response.
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status();

    /// @brief Asynchronously query account information (v1).
    /// @return A cobalt task yielding the account information response.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::account_information>>> async_account_status();

    /// @brief Query account information (v2).
    /// @return A result containing the v2 account information response.
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status_v2();

    /// @brief Asynchronously query account information (v2).
    /// @return A cobalt task yielding the v2 account information response.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::account_information>>> async_account_status_v2();

    /// @brief Query futures account balances.
    /// @return A result containing a vector of account balances.
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::futures_account_balance>>> account_balance();

    /// @brief Asynchronously query futures account balances.
    /// @return A cobalt task yielding a vector of account balances.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<std::vector<types::futures_account_balance>>>> async_account_balance();

    // -- Shared request type: position used by v1 and v2 --

    /// @brief Query position risk (v2) for one or all symbols.
    /// @param request Position query parameters (symbol filter).
    /// @return A result containing a vector of position risk entries.
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::position_risk>>>
    account_position_v2(const types::websocket_api_position_request& request);

    /// @brief Asynchronously query position risk (v2).
    /// @param request Position query parameters (symbol filter).
    /// @return A cobalt task yielding a vector of position risk entries.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<std::vector<types::position_risk>>>>
    async_account_position_v2(const types::websocket_api_position_request& request);

    // -- Shared request type: user_data_stream used by start/ping/stop --

    /// @brief Start a user data stream and obtain a listen key.
    /// @return A result containing the listen key.
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_start();

    /// @brief Asynchronously start a user data stream.
    /// @return A cobalt task yielding the listen key.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::websocket_api_listen_key_result>>> async_user_data_stream_start();

    /// @brief Ping (keep-alive) an existing user data stream.
    /// @return A result containing the listen key confirmation.
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_ping();

    /// @brief Asynchronously ping an existing user data stream.
    /// @return A cobalt task yielding the listen key confirmation.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::websocket_api_listen_key_result>>> async_user_data_stream_ping();

    /// @brief Stop (close) an existing user data stream.
    /// @return A result indicating the stream was closed.
    [[nodiscard]] result<types::websocket_api_response<types::empty_response_t>> user_data_stream_stop();

    /// @brief Asynchronously stop an existing user data stream.
    /// @return A cobalt task indicating the stream was closed.
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::empty_response_t>>> async_user_data_stream_stop();

    // -- Request type aliases for generic execute --

    using book_ticker_request = types::websocket_api_book_ticker_request;          ///< Alias for book ticker request type.
    using price_ticker_request = types::websocket_api_price_ticker_request;        ///< Alias for price ticker request type.
    using order_place_request = types::websocket_api_order_place_request;          ///< Alias for order placement request type.
    using order_query_request = types::websocket_api_order_query_request;          ///< Alias for order query request type.
    using order_cancel_request = types::websocket_api_order_cancel_request;        ///< Alias for order cancellation request type.
    using order_modify_request = types::websocket_api_order_modify_request;        ///< Alias for order modification request type.
    using position_request = types::websocket_api_position_request;                ///< Alias for position query request type.
    using algo_order_place_request = types::websocket_api_algo_order_place_request;  ///< Alias for algo order placement request type.
    using algo_order_cancel_request = types::websocket_api_algo_order_cancel_request; ///< Alias for algo order cancellation request type.

private:
    boost::asio::io_context& io_context_;
    config cfg_;
    transport::websocket_client transport_;

    /// @brief Generate a unique request ID for RPC correlation.
    /// @return A string ID derived from an incrementing counter.
    std::string next_id();
    std::uint64_t id_counter_{};

    /// @brief Serialize parameters, send an RPC call, and parse the response.
    /// @tparam Response The expected response payload type.
    /// @tparam Params   The parameter struct type to serialize.
    /// @param method RPC method name (e.g. "order.place").
    /// @param params The request parameters to serialize as JSON.
    /// @return A result containing the deserialized response.
    template<typename Response, typename Params>
    result<types::websocket_api_response<Response>>
    send_rpc(std::string_view method, const Params& params);

    /// @brief Build a base signed request populated with API key, timestamp, and signature.
    /// @return A signed request base suitable for authenticated endpoints.
    types::websocket_api_signed_request make_signed_request_base();

    /// @brief Inject authentication fields (API key, timestamp, signature) into a request.
    /// @tparam Request The request type to augment with auth fields.
    /// @param request The original request (credentials are merged in).
    /// @return A new request instance with authentication fields populated.
    template<class Request>
    Request inject_auth(const Request& request);
};

} // namespace binapi2::fapi::websocket_api
