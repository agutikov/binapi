// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/trade.hpp>

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

result<types::order_response>
trade_service::test_order(const types::test_new_order_request& request)
{
    return owner_.execute<types::order_response>(
        test_order_endpoint.method, std::string{ test_order_endpoint.path }, to_query_map(request), test_order_endpoint.signed_request);
}

boost::cobalt::task<result<types::order_response>>
trade_service::async_test_order(const types::new_order_request& request)
{
    co_return test_order(request);
}

result<std::vector<types::order_response>>
trade_service::batch_orders(const types::batch_orders_request& request)
{
    return owner_.execute<std::vector<types::order_response>>(
        batch_orders_endpoint.method, std::string{ batch_orders_endpoint.path }, to_query_map(request), batch_orders_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::order_response>>>
trade_service::async_batch_orders(const types::batch_orders_request& request)
{
    co_return batch_orders(request);
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

boost::cobalt::task<result<std::vector<types::order_response>>>
trade_service::async_modify_batch_orders(const types::batch_orders_request& request)
{
    co_return modify_batch_orders(request);
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

boost::cobalt::task<result<std::vector<types::order_response>>>
trade_service::async_cancel_batch_orders(const types::cancel_multiple_orders_request& request)
{
    co_return cancel_batch_orders(request);
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

boost::cobalt::task<result<std::vector<types::algo_order_response>>>
trade_service::async_open_algo_orders()
{
    co_return open_algo_orders();
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

boost::cobalt::task<result<types::code_msg_response>>
trade_service::async_cancel_all_algo_orders()
{
    co_return cancel_all_algo_orders();
}

result<types::code_msg_response>
trade_service::tradfi_perps(const types::tradfi_perps_request& /*request*/)
{
    query_map query{};
    return owner_.execute<types::code_msg_response>(
        tradfi_perps_endpoint.method, std::string{ tradfi_perps_endpoint.path }, query, tradfi_perps_endpoint.signed_request);
}

boost::cobalt::task<result<types::code_msg_response>>
trade_service::async_tradfi_perps(const types::tradfi_perps_request& request)
{
    co_return tradfi_perps(request);
}

} // namespace binapi2::fapi::rest
