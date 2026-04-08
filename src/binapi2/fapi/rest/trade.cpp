// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Trade methods with custom serialization that can't use generic execute.

#include <binapi2/fapi/rest/trade.hpp>

#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

// modify_batch_orders bypasses to_query_map because the batchOrders field
// is already a pre-serialized JSON array string that must be sent as-is.
boost::cobalt::task<result<std::vector<types::order_response_t>>>
trade_service::async_modify_batch_orders(const types::batch_orders_request_t& request)
{
    query_map query{ { "batchOrders", request.batchOrders } };
    co_return co_await pipeline_.async_execute<std::vector<types::order_response_t>>(
        modify_batch_orders_endpoint.method,
        std::string{ modify_batch_orders_endpoint.path },
        query,
        modify_batch_orders_endpoint.signed_request);
}

// cancel_batch_orders requires manual query construction because the Binance
// API expects orderIdList and origClientOrderIdList as JSON-formatted array
// strings within the query parameters.
boost::cobalt::task<result<std::vector<types::order_response_t>>>
trade_service::async_cancel_batch_orders(const types::cancel_multiple_orders_request_t& request)
{
    query_map query{ { "symbol", request.symbol } };
    if (request.orderIdList) {
        std::string ids;
        for (const auto& id : *request.orderIdList) {
            if (!ids.empty()) ids += ',';
            ids += std::to_string(id);
        }
        query["orderIdList"] = "[" + ids + "]";
    }
    if (request.origClientOrderIdList) {
        std::string ids;
        for (const auto& id : *request.origClientOrderIdList) {
            if (!ids.empty()) ids += ',';
            ids += "\"" + id + "\"";
        }
        query["origClientOrderIdList"] = "[" + ids + "]";
    }
    co_return co_await pipeline_.async_execute<std::vector<types::order_response_t>>(
        cancel_batch_orders_endpoint.method,
        std::string{ cancel_batch_orders_endpoint.path },
        query,
        cancel_batch_orders_endpoint.signed_request);
}

} // namespace binapi2::fapi::rest
