// SPDX-License-Identifier: Apache-2.0
//
// Convert command registry — authenticated asset conversion endpoints.
// Dispatches via the rest pipeline directly (convert_service has no
// generic async_execute).

#include "commands.hpp"

#include <binapi2/fapi/types/convert.hpp>

namespace demo_ui::rest {

namespace {

void cmd_convert_quote(const cmd_ctx& c)
{
    types::convert_quote_request_t req;
    req.fromAsset = c.form.from_asset;
    req.toAsset   = c.form.to_asset;
    if (!c.form.from_amount.empty())
        req.fromAmount = parse_decimal(c.form.from_amount);
    run_conv(c, std::move(req));
}

void cmd_convert_accept(const cmd_ctx& c)
{
    types::convert_accept_request_t req;
    req.quoteId = c.form.quote_id;
    run_conv(c, std::move(req));
}

void cmd_convert_order_status(const cmd_ctx& c)
{
    types::convert_order_status_request_t req;
    if (!c.form.convert_order_id.empty())
        req.orderId = c.form.convert_order_id;
    run_conv(c, std::move(req));
}

constexpr rest_command entries[] = {
    { "convert-quote",        "Convert quote <from> <to> <amount>", command_group::convert, form_kind::convert_quote_form,    &cmd_convert_quote },
    { "convert-accept",       "Accept convert quote <quoteId>",     command_group::convert, form_kind::quote_id_form,         &cmd_convert_accept },
    { "convert-order-status", "Convert order status <orderId>",     command_group::convert, form_kind::convert_order_id_form, &cmd_convert_order_status },
};

} // namespace

std::span<const rest_command> convert_commands()
{
    return { entries, sizeof(entries) / sizeof(entries[0]) };
}

} // namespace demo_ui::rest
