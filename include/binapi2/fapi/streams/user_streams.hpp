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
    using account_config_update_handler = std::function<bool(const types::account_config_update_event&)>;
    using trade_lite_handler = std::function<bool(const types::trade_lite_event&)>;
    using algo_order_update_handler = std::function<bool(const types::algo_order_update_event&)>;
    using conditional_order_reject_handler = std::function<bool(const types::conditional_order_trigger_reject_event&)>;
    using grid_update_handler = std::function<bool(const types::grid_update_event&)>;
    using strategy_update_handler = std::function<bool(const types::strategy_update_event&)>;

    struct handlers
    {
        account_update_handler on_account_update{};
        order_trade_update_handler on_order_trade_update{};
        margin_call_handler on_margin_call{};
        listen_key_expired_handler on_listen_key_expired{};
        account_config_update_handler on_account_config_update{};
        trade_lite_handler on_trade_lite{};
        algo_order_update_handler on_algo_order_update{};
        conditional_order_reject_handler on_conditional_order_reject{};
        grid_update_handler on_grid_update{};
        strategy_update_handler on_strategy_update{};
    };

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
    [[nodiscard]] result<void> read_loop(const handlers& h);
    void read_loop(const handlers& h, void_callback callback);
    [[nodiscard]] result<void> close();
    void close(void_callback callback);

private:
    boost::asio::io_context& io_context_;
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
