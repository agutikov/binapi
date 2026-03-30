#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/result.hpp>
#include <binapi2/umf/transport/websocket_client.hpp>
#include <binapi2/umf/types/websocket_api.hpp>

#include <boost/asio/io_context.hpp>

#include <string>

namespace binapi2::umf::websocket_api {

class client {
  public:
    client(boost::asio::io_context &io_context, config cfg);

    [[nodiscard]] result<void> connect();
    [[nodiscard]] result<types::websocket_api_response<types::session_logon_result>> session_logon();
    [[nodiscard]] result<types::websocket_api_response<types::account_information>> account_status();
    [[nodiscard]] result<types::websocket_api_response<std::vector<types::futures_account_balance>>> account_balance();
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> new_order(const types::new_order_request &request);
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> query_order(const types::query_order_request &request);
    [[nodiscard]] result<types::websocket_api_response<types::order_response>> cancel_order(const types::cancel_order_request &request);
    [[nodiscard]] result<types::websocket_api_response<types::book_ticker>> book_ticker(const types::book_ticker_request &request = {});
    [[nodiscard]] result<void> close();

  private:
    boost::asio::io_context &io_context_;
    config cfg_;
    transport::websocket_client transport_;
    std::string next_id();
    std::uint64_t id_counter_{};
};

} // namespace binapi2::umf::websocket_api
