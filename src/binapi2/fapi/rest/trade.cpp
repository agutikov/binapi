// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements trade REST endpoints that require special serialization
/// beyond what the generic execute path provides. Notably, cancel_batch_orders
/// needs custom query construction because the Binance API expects the order
/// ID lists as JSON array strings (e.g. orderIdList=[123,456]) rather than
/// the usual key=value form. Batch order placement and modification also use
/// a pre-serialized batchOrders JSON string.

#include <binapi2/fapi/rest/trade.hpp>

#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

boost::cobalt::task<result<types::order_response_t>>
trade_service::async_test_order(const types::new_order_request_t& request)
{
    co_return co_await pipeline_.async_execute<types::order_response_t>(
        test_order_endpoint.method, std::string{ test_order_endpoint.path }, to_query_map(request), test_order_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::order_response_t>>>
trade_service::async_batch_orders(const types::batch_orders_request_t& request)
{
    co_return co_await pipeline_.async_execute<std::vector<types::order_response_t>>(
        batch_orders_endpoint.method, std::string{ batch_orders_endpoint.path }, to_query_map(request), batch_orders_endpoint.signed_request);
}

// modify_batch_orders bypasses to_query_map because the batchOrders field
// is already a pre-serialized JSON array string that must be sent as-is.
boost::cobalt::task<result<std::vector<types::order_response_t>>>
trade_service::async_modify_batch_orders(const types::batch_orders_request_t& request)
{
    query_map query{ { "batchOrders", request.batchOrders } };
    co_return co_await pipeline_.async_execute<std::vector<types::order_response_t>>(modify_batch_orders_endpoint.method,
                                                              std::string{ modify_batch_orders_endpoint.path },
                                                              query,
                                                              modify_batch_orders_endpoint.signed_request);
}

// cancel_batch_orders requires manual query construction because the Binance
// API expects orderIdList and origClientOrderIdList as JSON-formatted array
// strings within the query parameters, e.g. orderIdList=[123,456] and
// origClientOrderIdList=["abc","def"]. The standard to_query_map serialization
// cannot produce this format, so the arrays are built by hand.
boost::cobalt::task<result<std::vector<types::order_response_t>>>
trade_service::async_cancel_batch_orders(const types::cancel_multiple_orders_request_t& request)
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
    co_return co_await pipeline_.async_execute<std::vector<types::order_response_t>>(cancel_batch_orders_endpoint.method,
                                                              std::string{ cancel_batch_orders_endpoint.path },
                                                              query,
                                                              cancel_batch_orders_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::algo_order_response_t>>>
trade_service::async_open_algo_orders()
{
    query_map query{};
    co_return co_await pipeline_.async_execute<std::vector<types::algo_order_response_t>>(open_algo_orders_endpoint.method,
                                                                   std::string{ open_algo_orders_endpoint.path },
                                                                   query,
                                                                   open_algo_orders_endpoint.signed_request);
}

boost::cobalt::task<result<types::code_msg_response_t>>
trade_service::async_cancel_all_algo_orders()
{
    query_map query{};
    co_return co_await pipeline_.async_execute<types::code_msg_response_t>(cancel_all_algo_orders_endpoint.method,
                                                    std::string{ cancel_all_algo_orders_endpoint.path },
                                                    query,
                                                    cancel_all_algo_orders_endpoint.signed_request);
}

boost::cobalt::task<result<types::code_msg_response_t>>
trade_service::async_tradfi_perps(const types::tradfi_perps_request_t& /*request*/)
{
    query_map query{};
    co_return co_await pipeline_.async_execute<types::code_msg_response_t>(
        tradfi_perps_endpoint.method, std::string{ tradfi_perps_endpoint.path }, query, tradfi_perps_endpoint.signed_request);
}

} // namespace binapi2::fapi::rest
