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
#include <boost/cobalt/task.hpp>

#include <string>
#include <type_traits>

namespace binapi2::fapi::websocket_api {

class client
{
public:
    client(boost::asio::io_context& io_context, config cfg);

    // Connection.
    [[nodiscard]] result<void> connect();
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect();
    [[nodiscard]] result<void> close();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    // Generic execute for request types with ws endpoint traits.
    template<class Request>
        requires has_ws_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request)
        -> result<types::websocket_api_response<typename endpoint_traits<Request>::response_type>>;

    template<class Request>
        requires has_ws_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<types::websocket_api_response<typename endpoint_traits<Request>::response_type>>>;

    // Session logon (unique auth flow).
    [[nodiscard]] result<types::websocket_api_response<types::session_logon_result>> session_logon();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::session_logon_result>>> async_session_logon();

    // Parameterless signed endpoints.
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::account_information>>> async_account_status();
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status_v2();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::account_information>>> async_account_status_v2();
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::futures_account_balance>>> account_balance();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<std::vector<types::futures_account_balance>>>> async_account_balance();

    // Shared request type: position used by v1 and v2.
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::position_risk>>>
    account_position_v2(const types::websocket_api_position_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<std::vector<types::position_risk>>>>
    async_account_position_v2(const types::websocket_api_position_request& request);

    // Shared request type: user_data_stream used by start/ping/stop.
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_start();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::websocket_api_listen_key_result>>> async_user_data_stream_start();
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_ping();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::websocket_api_listen_key_result>>> async_user_data_stream_ping();
    [[nodiscard]] result<types::websocket_api_response<types::empty_response>> user_data_stream_stop();
    [[nodiscard]] boost::cobalt::task<result<types::websocket_api_response<types::empty_response>>> async_user_data_stream_stop();

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

    template<typename Response, typename Params>
    result<types::websocket_api_response<Response>>
    send_rpc(std::string_view method, const Params& params);

    types::websocket_api_signed_request make_signed_request_base();

    template<class Request>
    Request inject_auth(const Request& request);
};

} // namespace binapi2::fapi::websocket_api
