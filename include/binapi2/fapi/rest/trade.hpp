// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <functional>
#include <vector>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class trade_service
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    explicit trade_service(client& owner) noexcept;

    [[nodiscard]] result<types::order_response> new_order(const types::new_order_request& request);
    void new_order(const types::new_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> modify_order(const types::modify_order_request& request);
    void modify_order(const types::modify_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> cancel_order(const types::cancel_order_request& request);
    void cancel_order(const types::cancel_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> query_order(const types::query_order_request& request);
    void query_order(const types::query_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> test_order(const types::test_new_order_request& request);
    void test_order(const types::test_new_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<std::vector<types::order_response>> batch_orders(const types::batch_orders_request& request);
    void batch_orders(const types::batch_orders_request& request, callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::order_response>> modify_batch_orders(const types::batch_orders_request& request);
    void modify_batch_orders(const types::batch_orders_request& request, callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::order_response>> cancel_batch_orders(const types::cancel_multiple_orders_request& request);
    void cancel_batch_orders(const types::cancel_multiple_orders_request& request,
                             callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<types::code_msg_response> cancel_all_open_orders(const types::cancel_all_open_orders_request& request);
    void cancel_all_open_orders(const types::cancel_all_open_orders_request& request,
                                callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::auto_cancel_response> auto_cancel(const types::auto_cancel_request& request);
    void auto_cancel(const types::auto_cancel_request& request, callback_type<types::auto_cancel_response> callback);
    [[nodiscard]] result<types::order_response> query_open_order(const types::query_open_order_request& request);
    void query_open_order(const types::query_open_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<std::vector<types::order_response>> all_open_orders(const types::all_open_orders_request& request);
    void all_open_orders(const types::all_open_orders_request& request,
                         callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::order_response>> all_orders(const types::all_orders_request& request);
    void all_orders(const types::all_orders_request& request, callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::position_risk_v3>> position_risk_v3(const types::position_info_v3_request& request);
    void position_risk_v3(const types::position_info_v3_request& request,
                          callback_type<std::vector<types::position_risk_v3>> callback);
    [[nodiscard]] result<std::vector<types::adl_quantile_entry>> adl_quantile(const types::adl_quantile_request& request);
    void adl_quantile(const types::adl_quantile_request& request,
                      callback_type<std::vector<types::adl_quantile_entry>> callback);
    [[nodiscard]] result<std::vector<types::order_response>> force_orders(const types::force_orders_request& request);
    void force_orders(const types::force_orders_request& request, callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<std::vector<types::account_trade_entry>> account_trades(const types::account_trade_request& request);
    void account_trades(const types::account_trade_request& request,
                        callback_type<std::vector<types::account_trade_entry>> callback);
    [[nodiscard]] result<types::code_msg_response> change_position_mode(const types::change_position_mode_request& request);
    void change_position_mode(const types::change_position_mode_request& request,
                              callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::code_msg_response> change_multi_assets(const types::change_multi_assets_mode_request& request);
    void change_multi_assets(const types::change_multi_assets_mode_request& request,
                             callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::change_leverage_response> change_leverage(const types::change_leverage_request& request);
    void change_leverage(const types::change_leverage_request& request,
                         callback_type<types::change_leverage_response> callback);
    [[nodiscard]] result<types::code_msg_response> change_margin_type(const types::change_margin_type_request& request);
    void change_margin_type(const types::change_margin_type_request& request,
                            callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::modify_isolated_margin_response>
    modify_isolated_margin(const types::modify_isolated_margin_request& request);
    void modify_isolated_margin(const types::modify_isolated_margin_request& request,
                                callback_type<types::modify_isolated_margin_response> callback);
    [[nodiscard]] result<std::vector<types::position_margin_history_entry>>
    position_margin_history(const types::position_margin_history_request& request);
    void position_margin_history(const types::position_margin_history_request& request,
                                 callback_type<std::vector<types::position_margin_history_entry>> callback);
    [[nodiscard]] result<std::vector<types::order_response>>
    order_modify_history(const types::order_modify_history_request& request);
    void order_modify_history(const types::order_modify_history_request& request,
                              callback_type<std::vector<types::order_response>> callback);
    [[nodiscard]] result<types::algo_order_response> new_algo_order(const types::new_algo_order_request& request);
    void new_algo_order(const types::new_algo_order_request& request, callback_type<types::algo_order_response> callback);
    [[nodiscard]] result<types::code_msg_response> cancel_algo_order(const types::cancel_algo_order_request& request);
    void cancel_algo_order(const types::cancel_algo_order_request& request, callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::algo_order_response> query_algo_order(const types::query_algo_order_request& request);
    void query_algo_order(const types::query_algo_order_request& request, callback_type<types::algo_order_response> callback);
    [[nodiscard]] result<std::vector<types::algo_order_response>> all_algo_orders(const types::all_algo_orders_request& request);
    void all_algo_orders(const types::all_algo_orders_request& request,
                         callback_type<std::vector<types::algo_order_response>> callback);
    [[nodiscard]] result<std::vector<types::algo_order_response>> open_algo_orders();
    void open_algo_orders(callback_type<std::vector<types::algo_order_response>> callback);
    [[nodiscard]] result<types::code_msg_response> cancel_all_algo_orders();
    void cancel_all_algo_orders(callback_type<types::code_msg_response> callback);
    [[nodiscard]] result<types::code_msg_response> tradfi_perps(const types::tradfi_perps_request& request = {});
    void tradfi_perps(const types::tradfi_perps_request& request, callback_type<types::code_msg_response> callback);

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
