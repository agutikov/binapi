// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/trade.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::fapi::rest {

trade_service::trade_service(binapi2::fapi::client& owner) noexcept : owner_(owner) {}

result<types::order_response>
trade_service::new_order(const types::new_order_request& request)
{
    query_map query{ { "symbol", request.symbol },
                     { "side", to_string(request.side) },
                     { "type", to_string(request.type) },
                     { "quantity", request.quantity } };
    if (request.timeInForce)
        query["timeInForce"] = to_string(*request.timeInForce);
    if (request.price)
        query["price"] = *request.price;
    if (request.newClientOrderId)
        query["newClientOrderId"] = *request.newClientOrderId;
    if (request.stopPrice)
        query["stopPrice"] = *request.stopPrice;
    return owner_.execute<types::order_response>(
        new_order_endpoint.method, std::string{ new_order_endpoint.path }, query, new_order_endpoint.signed_request);
}

void
trade_service::new_order(const types::new_order_request& request, callback_type<types::order_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(new_order(request)); });
}

result<types::order_response>
trade_service::modify_order(const types::modify_order_request& request)
{
    query_map query{
        { "symbol", request.symbol },
        { "side", to_string(request.side) },
        { "quantity", request.quantity },
        { "price", request.price },
    };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId)
        query["origClientOrderId"] = *request.origClientOrderId;
    if (request.priceMatch)
        query["priceMatch"] = *request.priceMatch;
    return owner_.execute<types::order_response>(
        modify_order_endpoint.method, std::string{ modify_order_endpoint.path }, query, modify_order_endpoint.signed_request);
}

void
trade_service::modify_order(const types::modify_order_request& request, callback_type<types::order_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(modify_order(request)); });
}

result<types::order_response>
trade_service::cancel_order(const types::cancel_order_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId)
        query["origClientOrderId"] = *request.origClientOrderId;
    return owner_.execute<types::order_response>(
        cancel_order_endpoint.method, std::string{ cancel_order_endpoint.path }, query, cancel_order_endpoint.signed_request);
}

void
trade_service::cancel_order(const types::cancel_order_request& request, callback_type<types::order_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(cancel_order(request)); });
}

result<types::order_response>
trade_service::query_order(const types::query_order_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId)
        query["origClientOrderId"] = *request.origClientOrderId;
    return owner_.execute<types::order_response>(
        query_order_endpoint.method, std::string{ query_order_endpoint.path }, query, query_order_endpoint.signed_request);
}

void
trade_service::query_order(const types::query_order_request& request, callback_type<types::order_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(query_order(request)); });
}

result<types::order_response>
trade_service::test_order(const types::test_new_order_request& request)
{
    query_map query{ { "symbol", request.symbol },
                     { "side", to_string(request.side) },
                     { "type", to_string(request.type) },
                     { "quantity", request.quantity } };
    if (request.timeInForce)
        query["timeInForce"] = to_string(*request.timeInForce);
    if (request.price)
        query["price"] = *request.price;
    if (request.newClientOrderId)
        query["newClientOrderId"] = *request.newClientOrderId;
    if (request.stopPrice)
        query["stopPrice"] = *request.stopPrice;
    return owner_.execute<types::order_response>(
        test_order_endpoint.method, std::string{ test_order_endpoint.path }, query, test_order_endpoint.signed_request);
}

void
trade_service::test_order(const types::test_new_order_request& request, callback_type<types::order_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(test_order(request)); });
}

result<std::vector<types::order_response>>
trade_service::batch_orders(const types::batch_orders_request& request)
{
    query_map query{ { "batchOrders", request.batchOrders } };
    return owner_.execute<std::vector<types::order_response>>(
        batch_orders_endpoint.method, std::string{ batch_orders_endpoint.path }, query, batch_orders_endpoint.signed_request);
}

void
trade_service::batch_orders(const types::batch_orders_request& request,
                            callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(batch_orders(request)); });
}

result<std::vector<types::order_response>>
trade_service::modify_batch_orders(const types::batch_orders_request& request)
{
    query_map query{ { "batchOrders", request.batchOrders } };
    return owner_.execute<std::vector<types::order_response>>(modify_batch_orders_endpoint.method,
                                                              std::string{ modify_batch_orders_endpoint.path },
                                                              query,
                                                              modify_batch_orders_endpoint.signed_request);
}

void
trade_service::modify_batch_orders(const types::batch_orders_request& request,
                                   callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(modify_batch_orders(request)); });
}

result<std::vector<types::order_response>>
trade_service::cancel_batch_orders(const types::cancel_multiple_orders_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderIdList) {
        std::string ids;
        for (const auto& id : *request.orderIdList) {
            if (!ids.empty())
                ids += ',';
            ids += std::to_string(id);
        }
        query["orderIdList"] = "[" + ids + "]";
    }
    if (request.origClientOrderIdList) {
        std::string ids;
        for (const auto& id : *request.origClientOrderIdList) {
            if (!ids.empty())
                ids += ',';
            ids += "\"" + id + "\"";
        }
        query["origClientOrderIdList"] = "[" + ids + "]";
    }
    return owner_.execute<std::vector<types::order_response>>(cancel_batch_orders_endpoint.method,
                                                              std::string{ cancel_batch_orders_endpoint.path },
                                                              query,
                                                              cancel_batch_orders_endpoint.signed_request);
}

void
trade_service::cancel_batch_orders(const types::cancel_multiple_orders_request& request,
                                   callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(cancel_batch_orders(request)); });
}

result<types::code_msg_response>
trade_service::cancel_all_open_orders(const types::cancel_all_open_orders_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    return owner_.execute<types::code_msg_response>(cancel_all_open_orders_endpoint.method,
                                                    std::string{ cancel_all_open_orders_endpoint.path },
                                                    query,
                                                    cancel_all_open_orders_endpoint.signed_request);
}

void
trade_service::cancel_all_open_orders(const types::cancel_all_open_orders_request& request,
                                      callback_type<types::code_msg_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(cancel_all_open_orders(request)); });
}

result<types::auto_cancel_response>
trade_service::auto_cancel(const types::auto_cancel_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "countdownTime", std::to_string(request.countdownTime) } };
    return owner_.execute<types::auto_cancel_response>(
        auto_cancel_endpoint.method, std::string{ auto_cancel_endpoint.path }, query, auto_cancel_endpoint.signed_request);
}

void
trade_service::auto_cancel(const types::auto_cancel_request& request, callback_type<types::auto_cancel_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(auto_cancel(request)); });
}

result<types::order_response>
trade_service::query_open_order(const types::query_open_order_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId)
        query["origClientOrderId"] = *request.origClientOrderId;
    return owner_.execute<types::order_response>(query_open_order_endpoint.method,
                                                 std::string{ query_open_order_endpoint.path },
                                                 query,
                                                 query_open_order_endpoint.signed_request);
}

void
trade_service::query_open_order(const types::query_open_order_request& request, callback_type<types::order_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(query_open_order(request)); });
}

result<std::vector<types::order_response>>
trade_service::all_open_orders(const types::all_open_orders_request& request)
{
    query_map query{};
    if (request.symbol)
        query["symbol"] = *request.symbol;
    return owner_.execute<std::vector<types::order_response>>(all_open_orders_endpoint.method,
                                                              std::string{ all_open_orders_endpoint.path },
                                                              query,
                                                              all_open_orders_endpoint.signed_request);
}

void
trade_service::all_open_orders(const types::all_open_orders_request& request,
                               callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(all_open_orders(request)); });
}

result<std::vector<types::order_response>>
trade_service::all_orders(const types::all_orders_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.startTime)
        query["startTime"] = std::to_string(*request.startTime);
    if (request.endTime)
        query["endTime"] = std::to_string(*request.endTime);
    if (request.limit)
        query["limit"] = std::to_string(*request.limit);
    return owner_.execute<std::vector<types::order_response>>(
        all_orders_endpoint.method, std::string{ all_orders_endpoint.path }, query, all_orders_endpoint.signed_request);
}

void
trade_service::all_orders(const types::all_orders_request& request,
                          callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(all_orders(request)); });
}

result<std::vector<types::position_risk_v3>>
trade_service::position_risk_v3(const types::position_info_v3_request& request)
{
    query_map query{};
    if (request.symbol)
        query["symbol"] = *request.symbol;
    return owner_.execute<std::vector<types::position_risk_v3>>(position_risk_v3_endpoint.method,
                                                                std::string{ position_risk_v3_endpoint.path },
                                                                query,
                                                                position_risk_v3_endpoint.signed_request);
}

void
trade_service::position_risk_v3(const types::position_info_v3_request& request,
                                callback_type<std::vector<types::position_risk_v3>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(position_risk_v3(request)); });
}

result<std::vector<types::adl_quantile_entry>>
trade_service::adl_quantile(const types::adl_quantile_request& request)
{
    query_map query{};
    if (request.symbol)
        query["symbol"] = *request.symbol;
    return owner_.execute<std::vector<types::adl_quantile_entry>>(
        adl_quantile_endpoint.method, std::string{ adl_quantile_endpoint.path }, query, adl_quantile_endpoint.signed_request);
}

void
trade_service::adl_quantile(const types::adl_quantile_request& request,
                             callback_type<std::vector<types::adl_quantile_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(adl_quantile(request)); });
}

result<std::vector<types::order_response>>
trade_service::force_orders(const types::force_orders_request& request)
{
    query_map query{};
    if (request.symbol)
        query["symbol"] = *request.symbol;
    if (request.autoCloseType)
        query["autoCloseType"] = *request.autoCloseType;
    if (request.startTime)
        query["startTime"] = std::to_string(*request.startTime);
    if (request.endTime)
        query["endTime"] = std::to_string(*request.endTime);
    if (request.limit)
        query["limit"] = std::to_string(*request.limit);
    return owner_.execute<std::vector<types::order_response>>(
        force_orders_endpoint.method, std::string{ force_orders_endpoint.path }, query, force_orders_endpoint.signed_request);
}

void
trade_service::force_orders(const types::force_orders_request& request,
                            callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(force_orders(request)); });
}

result<std::vector<types::account_trade_entry>>
trade_service::account_trades(const types::account_trade_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.startTime)
        query["startTime"] = std::to_string(*request.startTime);
    if (request.endTime)
        query["endTime"] = std::to_string(*request.endTime);
    if (request.fromId)
        query["fromId"] = std::to_string(*request.fromId);
    if (request.limit)
        query["limit"] = std::to_string(*request.limit);
    return owner_.execute<std::vector<types::account_trade_entry>>(account_trades_endpoint.method,
                                                                   std::string{ account_trades_endpoint.path },
                                                                   query,
                                                                   account_trades_endpoint.signed_request);
}

void
trade_service::account_trades(const types::account_trade_request& request,
                              callback_type<std::vector<types::account_trade_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(account_trades(request)); });
}

result<types::code_msg_response>
trade_service::change_position_mode(const types::change_position_mode_request& request)
{
    query_map query{ { "dualSidePosition", request.dualSidePosition } };
    return owner_.execute<types::code_msg_response>(change_position_mode_endpoint.method,
                                                    std::string{ change_position_mode_endpoint.path },
                                                    query,
                                                    change_position_mode_endpoint.signed_request);
}

void
trade_service::change_position_mode(const types::change_position_mode_request& request,
                                    callback_type<types::code_msg_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(change_position_mode(request)); });
}

result<types::code_msg_response>
trade_service::change_multi_assets(const types::change_multi_assets_mode_request& request)
{
    query_map query{ { "multiAssetsMargin", request.multiAssetsMargin } };
    return owner_.execute<types::code_msg_response>(change_multi_assets_endpoint.method,
                                                    std::string{ change_multi_assets_endpoint.path },
                                                    query,
                                                    change_multi_assets_endpoint.signed_request);
}

void
trade_service::change_multi_assets(const types::change_multi_assets_mode_request& request,
                                   callback_type<types::code_msg_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(change_multi_assets(request)); });
}

result<types::change_leverage_response>
trade_service::change_leverage(const types::change_leverage_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "leverage", std::to_string(request.leverage) } };
    return owner_.execute<types::change_leverage_response>(change_leverage_endpoint.method,
                                                           std::string{ change_leverage_endpoint.path },
                                                           query,
                                                           change_leverage_endpoint.signed_request);
}

void
trade_service::change_leverage(const types::change_leverage_request& request,
                               callback_type<types::change_leverage_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(change_leverage(request)); });
}

result<types::code_msg_response>
trade_service::change_margin_type(const types::change_margin_type_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "marginType", request.marginType } };
    return owner_.execute<types::code_msg_response>(change_margin_type_endpoint.method,
                                                    std::string{ change_margin_type_endpoint.path },
                                                    query,
                                                    change_margin_type_endpoint.signed_request);
}

void
trade_service::change_margin_type(const types::change_margin_type_request& request,
                                  callback_type<types::code_msg_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(change_margin_type(request)); });
}

result<types::modify_isolated_margin_response>
trade_service::modify_isolated_margin(const types::modify_isolated_margin_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "amount", request.amount }, { "type", std::to_string(request.type) } };
    if (request.positionSide)
        query["positionSide"] = *request.positionSide;
    return owner_.execute<types::modify_isolated_margin_response>(modify_isolated_margin_endpoint.method,
                                                                  std::string{ modify_isolated_margin_endpoint.path },
                                                                  query,
                                                                  modify_isolated_margin_endpoint.signed_request);
}

void
trade_service::modify_isolated_margin(const types::modify_isolated_margin_request& request,
                                      callback_type<types::modify_isolated_margin_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(modify_isolated_margin(request)); });
}

result<std::vector<types::position_margin_history_entry>>
trade_service::position_margin_history(const types::position_margin_history_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.type)
        query["type"] = std::to_string(*request.type);
    if (request.startTime)
        query["startTime"] = std::to_string(*request.startTime);
    if (request.endTime)
        query["endTime"] = std::to_string(*request.endTime);
    if (request.limit)
        query["limit"] = std::to_string(*request.limit);
    return owner_.execute<std::vector<types::position_margin_history_entry>>(position_margin_history_endpoint.method,
                                                                            std::string{ position_margin_history_endpoint.path },
                                                                            query,
                                                                            position_margin_history_endpoint.signed_request);
}

void
trade_service::position_margin_history(const types::position_margin_history_request& request,
                                       callback_type<std::vector<types::position_margin_history_entry>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(position_margin_history(request)); });
}

result<std::vector<types::order_response>>
trade_service::order_modify_history(const types::order_modify_history_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderId)
        query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId)
        query["origClientOrderId"] = *request.origClientOrderId;
    if (request.startTime)
        query["startTime"] = std::to_string(*request.startTime);
    if (request.endTime)
        query["endTime"] = std::to_string(*request.endTime);
    if (request.limit)
        query["limit"] = std::to_string(*request.limit);
    return owner_.execute<std::vector<types::order_response>>(order_modify_history_endpoint.method,
                                                              std::string{ order_modify_history_endpoint.path },
                                                              query,
                                                              order_modify_history_endpoint.signed_request);
}

void
trade_service::order_modify_history(const types::order_modify_history_request& request,
                                    callback_type<std::vector<types::order_response>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(order_modify_history(request)); });
}

result<types::algo_order_response>
trade_service::new_algo_order(const types::new_algo_order_request& request)
{
    query_map query{ { "symbol", request.symbol },
                     { "side", request.side },
                     { "type", request.type },
                     { "quantity", request.quantity },
                     { "algoType", request.algoType } };
    if (request.positionSide)
        query["positionSide"] = *request.positionSide;
    if (request.timeInForce)
        query["timeInForce"] = *request.timeInForce;
    if (request.price)
        query["price"] = *request.price;
    if (request.triggerPrice)
        query["triggerPrice"] = *request.triggerPrice;
    if (request.workingType)
        query["workingType"] = *request.workingType;
    if (request.clientAlgoId)
        query["clientAlgoId"] = *request.clientAlgoId;
    return owner_.execute<types::algo_order_response>(new_algo_order_endpoint.method,
                                                      std::string{ new_algo_order_endpoint.path },
                                                      query,
                                                      new_algo_order_endpoint.signed_request);
}

void
trade_service::new_algo_order(const types::new_algo_order_request& request,
                              callback_type<types::algo_order_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(new_algo_order(request)); });
}

result<types::code_msg_response>
trade_service::cancel_algo_order(const types::cancel_algo_order_request& request)
{
    query_map query{};
    if (request.algoId)
        query["algoId"] = std::to_string(*request.algoId);
    if (request.clientAlgoId)
        query["clientAlgoId"] = *request.clientAlgoId;
    return owner_.execute<types::code_msg_response>(cancel_algo_order_endpoint.method,
                                                    std::string{ cancel_algo_order_endpoint.path },
                                                    query,
                                                    cancel_algo_order_endpoint.signed_request);
}

void
trade_service::cancel_algo_order(const types::cancel_algo_order_request& request,
                                 callback_type<types::code_msg_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(cancel_algo_order(request)); });
}

result<types::algo_order_response>
trade_service::query_algo_order(const types::query_algo_order_request& request)
{
    query_map query{};
    if (request.algoId)
        query["algoId"] = std::to_string(*request.algoId);
    if (request.clientAlgoId)
        query["clientAlgoId"] = *request.clientAlgoId;
    return owner_.execute<types::algo_order_response>(query_algo_order_endpoint.method,
                                                      std::string{ query_algo_order_endpoint.path },
                                                      query,
                                                      query_algo_order_endpoint.signed_request);
}

void
trade_service::query_algo_order(const types::query_algo_order_request& request,
                                callback_type<types::algo_order_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(query_algo_order(request)); });
}

result<std::vector<types::algo_order_response>>
trade_service::all_algo_orders(const types::all_algo_orders_request& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.algoId)
        query["algoId"] = std::to_string(*request.algoId);
    if (request.startTime)
        query["startTime"] = std::to_string(*request.startTime);
    if (request.endTime)
        query["endTime"] = std::to_string(*request.endTime);
    if (request.page)
        query["page"] = std::to_string(*request.page);
    if (request.limit)
        query["limit"] = std::to_string(*request.limit);
    return owner_.execute<std::vector<types::algo_order_response>>(all_algo_orders_endpoint.method,
                                                                   std::string{ all_algo_orders_endpoint.path },
                                                                   query,
                                                                   all_algo_orders_endpoint.signed_request);
}

void
trade_service::all_algo_orders(const types::all_algo_orders_request& request,
                               callback_type<std::vector<types::algo_order_response>> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(all_algo_orders(request)); });
}

result<std::vector<types::algo_order_response>>
trade_service::open_algo_orders()
{
    query_map query{};
    return owner_.execute<std::vector<types::algo_order_response>>(open_algo_orders_endpoint.method,
                                                                   std::string{ open_algo_orders_endpoint.path },
                                                                   query,
                                                                   open_algo_orders_endpoint.signed_request);
}

void
trade_service::open_algo_orders(callback_type<std::vector<types::algo_order_response>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(open_algo_orders()); });
}

result<types::code_msg_response>
trade_service::cancel_all_algo_orders()
{
    query_map query{};
    return owner_.execute<types::code_msg_response>(cancel_all_algo_orders_endpoint.method,
                                                    std::string{ cancel_all_algo_orders_endpoint.path },
                                                    query,
                                                    cancel_all_algo_orders_endpoint.signed_request);
}

void
trade_service::cancel_all_algo_orders(callback_type<types::code_msg_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, callback = std::move(callback)]() mutable { callback(cancel_all_algo_orders()); });
}

result<types::code_msg_response>
trade_service::tradfi_perps(const types::tradfi_perps_request& /*request*/)
{
    query_map query{};
    return owner_.execute<types::code_msg_response>(
        tradfi_perps_endpoint.method, std::string{ tradfi_perps_endpoint.path }, query, tradfi_perps_endpoint.signed_request);
}

void
trade_service::tradfi_perps(const types::tradfi_perps_request& request, callback_type<types::code_msg_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(tradfi_perps(request)); });
}

} // namespace binapi2::fapi::rest
