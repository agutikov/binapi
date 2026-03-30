#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/result.hpp>
#include <binapi2/umf/transport/websocket_client.hpp>
#include <binapi2/umf/types/streams.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>

namespace binapi2::umf::streams {

class user_streams {
  public:
    using account_update_handler = std::function<bool(const types::account_update_event &)>;
    using margin_call_handler = std::function<bool(const types::margin_call_event &)>;
    using listen_key_expired_handler = std::function<bool(const types::listen_key_expired_event &)>;
    using order_trade_update_handler = std::function<bool(const types::order_trade_update_event &)>;

    user_streams(boost::asio::io_context &io_context, config cfg);

    [[nodiscard]] result<void> connect(const std::string &listen_key);
    [[nodiscard]] result<void> read_loop(
        account_update_handler account_handler,
        order_trade_update_handler order_handler,
        margin_call_handler margin_handler = {},
        listen_key_expired_handler listen_key_expired = {}
    );
    [[nodiscard]] result<void> close();

  private:
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::umf::streams
