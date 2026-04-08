// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Trade service for USD-M Futures order management and position configuration.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <boost/cobalt/task.hpp>

#include <vector>

namespace binapi2::fapi::rest {

/// @brief Service group for order placement, modification, cancellation, and position
///        configuration endpoints.
///
/// Request types with a unique endpoint mapping (e.g. new_order_request_t,
/// cancel_order_request_t) are exposed as using declarations and dispatched through
/// the inherited execute/async_execute template methods.
///
/// Named methods are provided for:
///   - **test_order**: uses the same new_order_request_t type but routes to /order/test.
///   - **batch_orders / modify_batch_orders**: use batch_orders_request_t, which serializes
///     its order list as a JSON array in the "batchOrders" query parameter.
///   - **cancel_batch_orders**: uses cancel_multiple_orders_request_t with special
///     serialization -- order IDs are encoded as JSON arrays in query parameters
///     (orderIdList / origClientOrderIdList), not as standard query values.
///   - **Parameterless endpoints**: open_algo_orders(), cancel_all_algo_orders(), tradfi_perps().
///
/// All endpoints in this service require HMAC signing (trade or user_data security level).
class trade_service : public service
{
public:
    using service::service;

    // Request types with 1:1 endpoint mapping (use execute/async_execute).
    using new_order_request_t = types::new_order_request_t;
    using modify_order_request_t = types::modify_order_request_t;
    using cancel_order_request_t = types::cancel_order_request_t;
    using query_order_request_t = types::query_order_request_t;
    using cancel_all_open_orders_request_t = types::cancel_all_open_orders_request_t;
    using auto_cancel_request_t = types::auto_cancel_request_t;
    using query_open_order_request_t = types::query_open_order_request_t;
    using all_open_orders_request_t = types::all_open_orders_request_t;
    using all_orders_request_t = types::all_orders_request_t;
    using position_info_v3_request_t = types::position_info_v3_request_t;
    using adl_quantile_request_t = types::adl_quantile_request_t;
    using force_orders_request_t = types::force_orders_request_t;
    using account_trade_request_t = types::account_trade_request_t;
    using change_position_mode_request_t = types::change_position_mode_request_t;
    using change_multi_assets_mode_request_t = types::change_multi_assets_mode_request_t;
    using change_leverage_request_t = types::change_leverage_request_t;
    using change_margin_type_request_t = types::change_margin_type_request_t;
    using modify_isolated_margin_request_t = types::modify_isolated_margin_request_t;
    using position_margin_history_request_t = types::position_margin_history_request_t;
    using order_modify_history_request_t = types::order_modify_history_request_t;
    using new_algo_order_request_t = types::new_algo_order_request_t;
    using cancel_algo_order_request_t = types::cancel_algo_order_request_t;
    using query_algo_order_request_t = types::query_algo_order_request_t;
    using all_algo_orders_request_t = types::all_algo_orders_request_t;

    /// @brief Submit a test order (validated but not placed on the matching engine).
    /// @param request  Order parameters (same type as new_order; routed to /order/test).
    [[nodiscard]] result<types::order_response_t> test_order(const types::new_order_request_t& request);
    /// @brief Async variant of test_order.
    [[nodiscard]] boost::cobalt::task<result<types::order_response_t>> async_test_order(const types::new_order_request_t& request);

    /// @brief Place up to 5 orders in a single request.
    /// @param request  Batch of order parameters serialized as a JSON array in the query.
    [[nodiscard]] result<std::vector<types::order_response_t>> batch_orders(const types::batch_orders_request_t& request);
    /// @brief Async variant of batch_orders.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::order_response_t>>> async_batch_orders(const types::batch_orders_request_t& request);

    /// @brief Modify up to 5 orders in a single request.
    /// @param request  Batch of order modification parameters.
    [[nodiscard]] result<std::vector<types::order_response_t>> modify_batch_orders(const types::batch_orders_request_t& request);
    /// @brief Async variant of modify_batch_orders.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::order_response_t>>> async_modify_batch_orders(const types::batch_orders_request_t& request);

    /// @brief Cancel multiple orders by order ID or client order ID.
    ///
    /// Uses special query serialization: order IDs are encoded as JSON arrays
    /// in the orderIdList and origClientOrderIdList query parameters, rather than
    /// repeated key-value pairs.
    ///
    /// @param request  Lists of order IDs and/or client order IDs to cancel.
    [[nodiscard]] result<std::vector<types::order_response_t>> cancel_batch_orders(const types::cancel_multiple_orders_request_t& request);
    /// @brief Async variant of cancel_batch_orders.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::order_response_t>>> async_cancel_batch_orders(const types::cancel_multiple_orders_request_t& request);

    /// @brief Fetch all currently open algo (VP/TWAP) orders.
    [[nodiscard]] result<std::vector<types::algo_order_response_t>> open_algo_orders();
    /// @brief Async variant of open_algo_orders.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::algo_order_response_t>>> async_open_algo_orders();

    /// @brief Cancel all open algo orders.
    [[nodiscard]] result<types::code_msg_response_t> cancel_all_algo_orders();
    /// @brief Async variant of cancel_all_algo_orders.
    [[nodiscard]] boost::cobalt::task<result<types::code_msg_response_t>> async_cancel_all_algo_orders();

    /// @brief TradFi perpetual contract endpoint.
    /// @param request  Optional request parameters (defaults to empty).
    [[nodiscard]] result<types::code_msg_response_t> tradfi_perps(const types::tradfi_perps_request_t& request);
    /// @brief Async variant of tradfi_perps.
    [[nodiscard]] boost::cobalt::task<result<types::code_msg_response_t>> async_tradfi_perps(const types::tradfi_perps_request_t& request);
};

} // namespace binapi2::fapi::rest
