// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>
#include <string>

namespace binapi2::fapi::websocket_api {

class client
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    client(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] result<void> connect();
    void connect(callback_type<void> callback);
    [[nodiscard]] result<types::websocket_api_response<types::session_logon_result>> session_logon();
    void session_logon(callback_type<types::websocket_api_response<types::session_logon_result>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status();
    void account_status(callback_type<types::websocket_api_response<types::account_information>> callback);
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::futures_account_balance>>> account_balance();
    void account_balance(callback_type<types::websocket_api_response<std::vector<types::futures_account_balance>>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> new_order(
        const types::new_order_request& request);
    void new_order(const types::new_order_request& request,
                   callback_type<types::websocket_api_response<types::order_response>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> query_order(
        const types::query_order_request& request);
    void query_order(const types::query_order_request& request,
                     callback_type<types::websocket_api_response<types::order_response>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> cancel_order(
        const types::cancel_order_request& request);
    void cancel_order(const types::cancel_order_request& request,
                      callback_type<types::websocket_api_response<types::order_response>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::book_ticker>> book_ticker(
        const types::book_ticker_request& request = {});
    void book_ticker(const types::book_ticker_request& request,
                     callback_type<types::websocket_api_response<types::book_ticker>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::price_ticker>> ticker_price(
        const types::price_ticker_request& request = {});
    void ticker_price(const types::price_ticker_request& request,
                      callback_type<types::websocket_api_response<types::price_ticker>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> modify_order(
        const types::modify_order_request& request);
    void modify_order(const types::modify_order_request& request,
                      callback_type<types::websocket_api_response<types::order_response>> callback);
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::position_risk>>> account_position(
        const types::position_risk_request& request = {});
    void account_position(const types::position_risk_request& request,
                          callback_type<types::websocket_api_response<std::vector<types::position_risk>>> callback);
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::position_risk>>> account_position_v2(
        const types::position_risk_request& request = {});
    void account_position_v2(const types::position_risk_request& request,
                             callback_type<types::websocket_api_response<std::vector<types::position_risk>>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status_v2();
    void account_status_v2(callback_type<types::websocket_api_response<types::account_information>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::algo_order_response>> algo_order_place(
        const types::new_algo_order_request& request);
    void algo_order_place(const types::new_algo_order_request& request,
                          callback_type<types::websocket_api_response<types::algo_order_response>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::code_msg_response>> algo_order_cancel(
        const types::cancel_algo_order_request& request);
    void algo_order_cancel(const types::cancel_algo_order_request& request,
                           callback_type<types::websocket_api_response<types::code_msg_response>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_start();
    void user_data_stream_start(callback_type<types::websocket_api_response<types::websocket_api_listen_key_result>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::websocket_api_listen_key_result>> user_data_stream_ping();
    void user_data_stream_ping(callback_type<types::websocket_api_response<types::websocket_api_listen_key_result>> callback);
    [[nodiscard]] result<types::websocket_api_response<types::empty_response>> user_data_stream_stop();
    void user_data_stream_stop(callback_type<types::websocket_api_response<types::empty_response>> callback);
    [[nodiscard]] result<void> close();
    void close(callback_type<void> callback);

private:
    boost::asio::io_context& io_context_;
    config cfg_;
    transport::websocket_client transport_;
    std::string next_id();
    std::uint64_t id_counter_{};
};

} // namespace binapi2::fapi::websocket_api
