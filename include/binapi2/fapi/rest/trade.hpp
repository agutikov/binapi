// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <vector>

namespace binapi2::fapi::rest {

class trade_service : public service
{
public:
    using service::service;

    // Request types with 1:1 endpoint mapping (use execute/async_execute).
    using new_order_request = types::new_order_request;
    using modify_order_request = types::modify_order_request;
    using cancel_order_request = types::cancel_order_request;
    using query_order_request = types::query_order_request;
    using cancel_all_open_orders_request = types::cancel_all_open_orders_request;
    using auto_cancel_request = types::auto_cancel_request;
    using query_open_order_request = types::query_open_order_request;
    using all_open_orders_request = types::all_open_orders_request;
    using all_orders_request = types::all_orders_request;
    using position_info_v3_request = types::position_info_v3_request;
    using adl_quantile_request = types::adl_quantile_request;
    using force_orders_request = types::force_orders_request;
    using account_trade_request = types::account_trade_request;
    using change_position_mode_request = types::change_position_mode_request;
    using change_multi_assets_mode_request = types::change_multi_assets_mode_request;
    using change_leverage_request = types::change_leverage_request;
    using change_margin_type_request = types::change_margin_type_request;
    using modify_isolated_margin_request = types::modify_isolated_margin_request;
    using position_margin_history_request = types::position_margin_history_request;
    using order_modify_history_request = types::order_modify_history_request;
    using new_algo_order_request = types::new_algo_order_request;
    using cancel_algo_order_request = types::cancel_algo_order_request;
    using query_algo_order_request = types::query_algo_order_request;
    using all_algo_orders_request = types::all_algo_orders_request;

    // Methods for shared/special request types.
    [[nodiscard]] result<types::order_response> test_order(const types::new_order_request& request);
    void test_order(const types::new_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<std::vector<types::order_response>> batch_orders(const types::batch_orders_request& request);
    void batch_orders(const types::batch_orders_request& request, callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::order_response>> modify_batch_orders(const types::batch_orders_request& request);
    void modify_batch_orders(const types::batch_orders_request& request, callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::order_response>> cancel_batch_orders(const types::cancel_multiple_orders_request& request);
    void cancel_batch_orders(const types::cancel_multiple_orders_request& request,
                             callback_type<std::vector<types::order_response>> callback);

    // Parameterless endpoints.
    [[nodiscard]] result<std::vector<types::algo_order_response>> open_algo_orders();
    void open_algo_orders(callback_type<std::vector<types::algo_order_response>> callback);
    [[nodiscard]] result<types::code_msg_response> cancel_all_algo_orders();
    void cancel_all_algo_orders(callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::code_msg_response> tradfi_perps(const types::tradfi_perps_request& request = {});
    void tradfi_perps(const types::tradfi_perps_request& request, callback_type<types::code_msg_response> callback);
};

} // namespace binapi2::fapi::rest
