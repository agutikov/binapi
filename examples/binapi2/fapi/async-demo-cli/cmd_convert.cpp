// SPDX-License-Identifier: Apache-2.0
//
// Convert commands — authenticated asset conversion endpoints.

#include "cmd_convert.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_convert_quote(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) { spdlog::error("usage: convert-quote <fromAsset> <toAsset> <fromAmount>"); co_return 1; }
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::convert_quote_request_t req;
    req.fromAsset = args[0];
    req.toAsset = args[1];
    req.fromAmount = types::decimal_t(std::string(args[2]));
    auto r = co_await (*rest)->rest_pipeline().async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_convert_accept(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: convert-accept <quoteId>"); co_return 1; }
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::convert_accept_request_t req;
    req.quoteId = args[0];
    auto r = co_await (*rest)->rest_pipeline().async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_convert_order_status(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: convert-order-status <orderId>"); co_return 1; }
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::convert_order_status_request_t req;
    req.orderId = args[0];
    auto r = co_await (*rest)->rest_pipeline().async_execute(req);
    co_return handle_result(r);
}

} // namespace demo
