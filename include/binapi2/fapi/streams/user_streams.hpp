// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>

namespace binapi2::fapi::streams {

class user_streams
{
public:
    using void_callback = std::function<void(result<void>)>;
    using account_update_handler = std::function<bool(const types::account_update_event&)>;
    using margin_call_handler = std::function<bool(const types::margin_call_event&)>;
    using listen_key_expired_handler = std::function<bool(const types::listen_key_expired_event&)>;
    using order_trade_update_handler = std::function<bool(const types::order_trade_update_event&)>;

    user_streams(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] result<void> connect(const std::string& listen_key);
    void connect(const std::string& listen_key, void_callback callback);
    [[nodiscard]] result<void> read_loop(account_update_handler account_handler,
                                         order_trade_update_handler order_handler,
                                         margin_call_handler margin_handler = {},
                                         listen_key_expired_handler listen_key_expired = {});
    void read_loop(account_update_handler account_handler,
                   order_trade_update_handler order_handler,
                   margin_call_handler margin_handler,
                   listen_key_expired_handler listen_key_expired,
                   void_callback callback);
    [[nodiscard]] result<void> close();
    void close(void_callback callback);

private:
    boost::asio::io_context& io_context_;
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
