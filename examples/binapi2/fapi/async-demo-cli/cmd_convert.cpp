// SPDX-License-Identifier: Apache-2.0
//
// Convert commands — authenticated asset conversion endpoints.

#include "cmd_convert.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

/// Convert commands dispatch via the rest pipeline directly (convert_service
/// has no generic async_execute), so we roll a tiny sink-aware helper here
/// instead of using `lib::exec_*`. One-shot, three commands — not worth a
/// library layer.
template<typename Request>
boost::cobalt::task<int>
run_convert(binapi2::futures_usdm_api& c, Request req, lib::result_sink& sink)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
    auto r = co_await (*rest)->rest_pipeline().async_execute(req);
    if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
    if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
        sink.on_response_json(*j);
    sink.on_done(0);
    co_return 0;
}

} // namespace

void register_cmd_convert(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Convert";

    // convert-quote: <from> <to> <amount>
    {
        struct opts_t { std::string from_asset, to_asset, from_amount; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("convert-quote", "Convert quote <from> <to> <amount> (auth)");
        sub->group(group);
        sub->add_option("from",   opts->from_asset,  "From asset")->required();
        sub->add_option("to",     opts->to_asset,    "To asset")->required();
        sub->add_option("amount", opts->from_amount, "From amount")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::convert_quote_request_t req;
                req.fromAsset = opts->from_asset;
                req.toAsset = opts->to_asset;
                req.fromAmount = types::decimal_t(opts->from_amount);
                co_return co_await run_convert(c, std::move(req), sink);
            };
        });
    }

    // convert-accept: <quoteId>
    {
        struct opts_t { std::string quote_id; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("convert-accept", "Accept convert quote <quoteId> (auth)");
        sub->group(group);
        sub->add_option("quoteId", opts->quote_id, "Quote ID")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::convert_accept_request_t req;
                req.quoteId = opts->quote_id;
                co_return co_await run_convert(c, std::move(req), sink);
            };
        });
    }

    // convert-order-status: <orderId>
    {
        struct opts_t { std::string order_id; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("convert-order-status", "Convert order status <orderId> (auth)");
        sub->group(group);
        sub->add_option("orderId", opts->order_id, "Convert order ID")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::convert_order_status_request_t req;
                req.orderId = opts->order_id;
                co_return co_await run_convert(c, std::move(req), sink);
            };
        });
    }
}

} // namespace demo
