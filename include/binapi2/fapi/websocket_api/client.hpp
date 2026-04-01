// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/fapi/websocket_api/endpoint_traits.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>
#include <string>
#include <type_traits>

namespace binapi2::fapi::websocket_api {

class client
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    client(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] result<void> connect();
    void connect(callback_type<void> callback);
    [[nodiscard]] result<void> close();
    void close(callback_type<void> callback);

    // Generic execute for request types with ws endpoint traits.
    // Injects auth for signed requests (those inheriting from websocket_api_signed_request).
    template<class Request>
        requires has_ws_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request)
        -> result<types::websocket_api_response<typename endpoint_traits<Request>::response_type>>;

    template<class Request>
        requires has_ws_endpoint_traits<Request>
    void async_execute(
        const Request& request,
        callback_type<types::websocket_api_response<typename endpoint_traits<Request>::response_type>> callback);

    // Session logon (unique auth flow, not generic).
    [[nodiscard]] result<types::websocket_api_response<types::session_logon_result>> session_logon();
    void session_logon(callback_type<types::websocket_api_response<types::session_logon_result>> callback);

    // Parameterless signed endpoints.
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status();
    void account_status(callback_type<types::websocket_api_response<types::account_information>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status_v2();
    void account_status_v2(callback_type<types::websocket_api_response<types::account_information>> callback);
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::futures_account_balance>>> account_balance();
    void account_balance(callback_type<types::websocket_api_response<std::vector<types::futures_account_balance>>> callback);

    // Shared request type: position_risk_request used by v1 and v2.
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::position_risk>>>
    account_position_v2(const types::websocket_api_position_request& request);
    void account_position_v2(const types::websocket_api_position_request& request,
                             callback_type<types::websocket_api_response<std::vector<types::position_risk>>> callback);

    // Shared request type: user_data_stream used by start/ping/stop.
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_start();
    void user_data_stream_start(callback_type<types::websocket_api_response<types::websocket_api_listen_key_result>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_ping();
    void user_data_stream_ping(callback_type<types::websocket_api_response<types::websocket_api_listen_key_result>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::empty_response>> user_data_stream_stop();
    void user_data_stream_stop(callback_type<types::websocket_api_response<types::empty_response>> callback);

    // Request type aliases for generic execute.
    using book_ticker_request = types::websocket_api_book_ticker_request;
    using price_ticker_request = types::websocket_api_price_ticker_request;
    using order_place_request = types::websocket_api_order_place_request;
    using order_query_request = types::websocket_api_order_query_request;
    using order_cancel_request = types::websocket_api_order_cancel_request;
    using order_modify_request = types::websocket_api_order_modify_request;
    using position_request = types::websocket_api_position_request;
    using algo_order_place_request = types::websocket_api_algo_order_place_request;
    using algo_order_cancel_request = types::websocket_api_algo_order_cancel_request;

private:
    boost::asio::io_context& io_context_;
    config cfg_;
    transport::websocket_client transport_;
    std::string next_id();
    std::uint64_t id_counter_{};

    // Internal helpers.
    template<typename Response, typename Params>
    result<types::websocket_api_response<Response>>
    send_rpc(std::string_view method, const Params& params);

    types::websocket_api_signed_request make_signed_request_base();

    template<class Request>
    Request inject_auth(const Request& request);
};

} // namespace binapi2::fapi::websocket_api
