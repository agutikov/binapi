// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/account.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::fapi::rest {

account_service::account_service(binapi2::fapi::client& owner) noexcept : owner_(owner) {}

result<types::account_information>
account_service::account_information()
{
    return owner_.execute<types::account_information>(account_information_endpoint.method,
                                                      std::string{ account_information_endpoint.path },
                                                      {},
                                                      account_information_endpoint.signed_request);
}

void
account_service::account_information(callback_type<types::account_information> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(account_information()); });
}

result<std::vector<types::futures_account_balance>>
account_service::balances()
{
    return owner_.execute<std::vector<types::futures_account_balance>>(account_balances_endpoint.method,
                                                                       std::string{ account_balances_endpoint.path },
                                                                       {},
                                                                       account_balances_endpoint.signed_request);
}

void
account_service::balances(callback_type<std::vector<types::futures_account_balance>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(balances()); });
}

result<std::vector<types::position_risk>>
account_service::position_risk(const types::position_risk_request& request)
{
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<std::vector<types::position_risk>>(position_risk_endpoint.method,
                                                             std::string{ position_risk_endpoint.path },
                                                             query,
                                                             position_risk_endpoint.signed_request);
}

void
account_service::position_risk(const types::position_risk_request& request,
                               callback_type<std::vector<types::position_risk>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(position_risk(request)); });
}

} // namespace binapi2::fapi::rest
