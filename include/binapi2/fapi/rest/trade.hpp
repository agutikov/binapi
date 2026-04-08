// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Trade REST service — signed endpoints for orders, positions, algo.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/trade.hpp>

namespace binapi2::fapi::rest {

class trade_service : public service
{
public:
    using service::service;

    template<class Request>
        requires is_trade_request<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>
    {
        co_return co_await pipeline_.async_execute(request);
    }

    // --- Named methods for custom serialization (cannot use generic execute) ---

    /// @brief Place batch orders (pre-serialized batchOrders JSON array).
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::order_response_t>>>
    async_modify_batch_orders(const types::batch_orders_request_t& request);

    /// @brief Cancel batch orders (JSON array lists for orderIdList/origClientOrderIdList).
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::order_response_t>>>
    async_cancel_batch_orders(const types::cancel_multiple_orders_request_t& request);

    // --- Request type aliases ---

    using new_order_request = types::new_order_request_t;
    using test_order_request = types::test_order_request_t;
    using modify_order_request = types::modify_order_request_t;
    using cancel_order_request = types::cancel_order_request_t;
    using query_order_request = types::query_order_request_t;
    using batch_orders_request = types::batch_orders_request_t;
    using cancel_multiple_orders_request = types::cancel_multiple_orders_request_t;
    using cancel_all_open_orders_request = types::cancel_all_open_orders_request_t;
    using auto_cancel_request = types::auto_cancel_request_t;
    using query_open_order_request = types::query_open_order_request_t;
    using all_open_orders_request = types::all_open_orders_request_t;
    using all_orders_request = types::all_orders_request_t;
    using position_info_v3_request = types::position_info_v3_request_t;
    using adl_quantile_request = types::adl_quantile_request_t;
    using force_orders_request = types::force_orders_request_t;
    using account_trade_request = types::account_trade_request_t;
    using change_position_mode_request = types::change_position_mode_request_t;
    using change_multi_assets_mode_request = types::change_multi_assets_mode_request_t;
    using change_leverage_request = types::change_leverage_request_t;
    using change_margin_type_request = types::change_margin_type_request_t;
    using modify_isolated_margin_request = types::modify_isolated_margin_request_t;
    using position_margin_history_request = types::position_margin_history_request_t;
    using order_modify_history_request = types::order_modify_history_request_t;
    using new_algo_order_request = types::new_algo_order_request_t;
    using cancel_algo_order_request = types::cancel_algo_order_request_t;
    using query_algo_order_request = types::query_algo_order_request_t;
    using all_algo_orders_request = types::all_algo_orders_request_t;
    using open_algo_orders_request = types::open_algo_orders_request_t;
    using cancel_all_algo_orders_request = types::cancel_all_algo_orders_request_t;
    using tradfi_perps_request = types::tradfi_perps_request_t;
};

} // namespace binapi2::fapi::rest
