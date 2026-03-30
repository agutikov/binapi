#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/result.hpp>
#include <binapi2/umf/transport/websocket_client.hpp>
#include <binapi2/umf/types/websocket_api.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>
#include <string>

namespace binapi2::umf::websocket_api {

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
    void new_order(const types::new_order_request& request, callback_type<types::websocket_api_response<types::order_response>> callback);
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
    [[nodiscard]] result<void> close();
    void close(callback_type<void> callback);

private:
    boost::asio::io_context& io_context_;
    config cfg_;
    transport::websocket_client transport_;
    std::string next_id();
    std::uint64_t id_counter_{};
};

} // namespace binapi2::umf::websocket_api
