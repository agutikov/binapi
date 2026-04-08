// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_streams.hpp
/// @brief User data stream client for Binance USD-M Futures account events.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>

namespace binapi2::fapi::streams {

/// @brief Client for receiving real-time account and order events via the
///        Binance USD-M Futures user data stream.
///
/// A listen key (obtained from the REST or WebSocket API) is required to
/// connect. Once connected, call one of the @ref read_loop overloads with
/// event handler callbacks. Each handler returns @c true to continue reading
/// or @c false to terminate the loop.
class user_streams
{
public:
    using void_callback = std::function<void(result<void>)>;                                          ///< Async completion callback type.
    using account_update_handler = std::function<bool(const types::account_update_event_t&)>;            ///< Handler for account update events (balance/position changes).
    using margin_call_handler = std::function<bool(const types::margin_call_event_t&)>;                  ///< Handler for margin call warning events.
    using listen_key_expired_handler = std::function<bool(const types::listen_key_expired_event_t&)>;    ///< Handler for listen key expiration events.
    using order_trade_update_handler = std::function<bool(const types::order_trade_update_event_t&)>;    ///< Handler for order/trade update events.
    using account_config_update_handler = std::function<bool(const types::account_config_update_event_t&)>; ///< Handler for account configuration change events.
    using trade_lite_handler = std::function<bool(const types::trade_lite_event_t&)>;                    ///< Handler for lightweight trade events.
    using algo_order_update_handler = std::function<bool(const types::algo_order_update_event_t&)>;      ///< Handler for algorithmic order update events.
    using conditional_order_reject_handler = std::function<bool(const types::conditional_order_trigger_reject_event_t&)>; ///< Handler for conditional order trigger rejection events.
    using grid_update_handler = std::function<bool(const types::grid_update_event_t&)>;                  ///< Handler for grid strategy update events.
    using strategy_update_handler = std::function<bool(const types::strategy_update_event_t&)>;          ///< Handler for strategy update events.

    /// @brief Aggregate struct holding all user data stream event handlers.
    ///
    /// Pass an instance to @ref read_loop to handle all event types. Handlers
    /// left as default (empty) will cause the corresponding events to be silently
    /// skipped.
    struct handlers
    {
        account_update_handler on_account_update{};                ///< Called on balance/position changes.
        order_trade_update_handler on_order_trade_update{};        ///< Called on order fills and status changes.
        margin_call_handler on_margin_call{};                      ///< Called on margin call warnings.
        listen_key_expired_handler on_listen_key_expired{};        ///< Called when the listen key expires.
        account_config_update_handler on_account_config_update{};  ///< Called on leverage or margin type changes.
        trade_lite_handler on_trade_lite{};                        ///< Called for lightweight trade notifications.
        algo_order_update_handler on_algo_order_update{};          ///< Called on algo order status changes.
        conditional_order_reject_handler on_conditional_order_reject{}; ///< Called when a conditional order trigger is rejected.
        grid_update_handler on_grid_update{};                      ///< Called on grid strategy updates.
        strategy_update_handler on_strategy_update{};              ///< Called on strategy updates.
    };

    /// @brief Construct a user streams client.
    /// @param io  The io_thread that owns the io_context.
    /// @param cfg Configuration containing endpoint and credential settings.
    user_streams(detail::io_thread& io, config cfg);

    /// @brief Connect to the user data stream using a listen key.
    /// @param listen_key The listen key obtained from the REST or WebSocket API.
    /// @return A result indicating success or connection error.
    [[nodiscard]] result<void> connect(const std::string& listen_key);
    /// @brief Async overload with completion callback.
    void connect(const std::string& listen_key, void_callback callback);

    /// @brief Run the read loop with individual event handlers.
    ///
    /// Continuously reads events and dispatches them to the provided handlers.
    /// The loop terminates when any handler returns @c false or an error occurs.
    ///
    /// @param account_handler     Handler for account update events (required).
    /// @param order_handler       Handler for order/trade update events (required).
    /// @param margin_handler      Optional handler for margin call events.
    /// @param listen_key_expired  Optional handler for listen key expiration.
    /// @return A result indicating how the loop terminated.
    [[nodiscard]] result<void> read_loop(account_update_handler account_handler,
                                         order_trade_update_handler order_handler,
                                         margin_call_handler margin_handler = {},
                                         listen_key_expired_handler listen_key_expired = {});
    /// @brief Async overload with completion callback.
    void read_loop(account_update_handler account_handler,
                   order_trade_update_handler order_handler,
                   margin_call_handler margin_handler,
                   listen_key_expired_handler listen_key_expired,
                   void_callback callback);

    /// @brief Run the read loop with the aggregate handlers struct.
    ///
    /// Preferred overload when handling more than the four basic event types.
    ///
    /// @param h Struct containing all event handlers.
    /// @return A result indicating how the loop terminated.
    [[nodiscard]] result<void> read_loop(const handlers& h);
    /// @brief Async overload with completion callback.
    void read_loop(const handlers& h, void_callback callback);

    /// @brief Synchronously close the user data stream connection.
    [[nodiscard]] result<void> close();
    /// @brief Async overload with completion callback.
    void close(void_callback callback);

private:
    boost::asio::io_context& io_context_;
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
