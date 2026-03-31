#include <binapi2/umf/rest/trade.hpp>

#include <binapi2/umf/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::umf::rest {

trade_service::trade_service(binapi2::umf::client& owner) noexcept : owner_(owner) {}

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

} // namespace binapi2::umf::rest
