// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/account.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::fapi::rest {

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

result<types::account_config_response>
account_service::account_config()
{
    return owner_.execute<types::account_config_response>(account_config_endpoint.method,
                                                          std::string{ account_config_endpoint.path },
                                                          {},
                                                          account_config_endpoint.signed_request);
}

void
account_service::account_config(callback_type<types::account_config_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(account_config()); });
}

result<types::multi_assets_mode_response>
account_service::get_multi_assets_mode()
{
    return owner_.execute<types::multi_assets_mode_response>(get_multi_assets_mode_endpoint.method,
                                                             std::string{ get_multi_assets_mode_endpoint.path },
                                                             {},
                                                             get_multi_assets_mode_endpoint.signed_request);
}

void
account_service::get_multi_assets_mode(callback_type<types::multi_assets_mode_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(get_multi_assets_mode()); });
}

result<types::position_mode_response>
account_service::get_position_mode()
{
    return owner_.execute<types::position_mode_response>(get_position_mode_endpoint.method,
                                                         std::string{ get_position_mode_endpoint.path },
                                                         {},
                                                         get_position_mode_endpoint.signed_request);
}

void
account_service::get_position_mode(callback_type<types::position_mode_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(get_position_mode()); });
}

result<std::vector<types::rate_limit>>
account_service::rate_limit_order()
{
    return owner_.execute<std::vector<types::rate_limit>>(rate_limit_order_endpoint.method,
                                                          std::string{ rate_limit_order_endpoint.path },
                                                          {},
                                                          rate_limit_order_endpoint.signed_request);
}

void
account_service::rate_limit_order(callback_type<std::vector<types::rate_limit>> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(rate_limit_order()); });
}

result<types::bnb_burn_status_response>
account_service::get_bnb_burn()
{
    return owner_.execute<types::bnb_burn_status_response>(get_bnb_burn_endpoint.method,
                                                           std::string{ get_bnb_burn_endpoint.path },
                                                           {},
                                                           get_bnb_burn_endpoint.signed_request);
}

void
account_service::get_bnb_burn(callback_type<types::bnb_burn_status_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(get_bnb_burn()); });
}

result<types::download_id_response>
account_service::download_id_transaction(const types::download_id_request& request)
{
    return owner_.execute<types::download_id_response>(download_id_transaction_endpoint.method,
                                                       std::string{ download_id_transaction_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_transaction_endpoint.signed_request);
}

void
account_service::download_id_transaction(const types::download_id_request& request,
                                         callback_type<types::download_id_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(download_id_transaction(request)); });
}

result<types::download_link_response>
account_service::download_link_transaction(const types::download_link_request& request)
{
    return owner_.execute<types::download_link_response>(download_link_transaction_endpoint.method,
                                                         std::string{ download_link_transaction_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_transaction_endpoint.signed_request);
}

void
account_service::download_link_transaction(const types::download_link_request& request,
                                           callback_type<types::download_link_response> callback)
{
    detail::post_callback(
        owner_.context(),
        [this, request, callback = std::move(callback)]() mutable { callback(download_link_transaction(request)); });
}

result<types::download_id_response>
account_service::download_id_order(const types::download_id_request& request)
{
    return owner_.execute<types::download_id_response>(download_id_order_endpoint.method,
                                                       std::string{ download_id_order_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_order_endpoint.signed_request);
}

void
account_service::download_id_order(const types::download_id_request& request,
                                   callback_type<types::download_id_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(download_id_order(request)); });
}

result<types::download_link_response>
account_service::download_link_order(const types::download_link_request& request)
{
    return owner_.execute<types::download_link_response>(download_link_order_endpoint.method,
                                                         std::string{ download_link_order_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_order_endpoint.signed_request);
}

void
account_service::download_link_order(const types::download_link_request& request,
                                     callback_type<types::download_link_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(download_link_order(request)); });
}

result<types::download_id_response>
account_service::download_id_trade(const types::download_id_request& request)
{
    return owner_.execute<types::download_id_response>(download_id_trade_endpoint.method,
                                                       std::string{ download_id_trade_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_trade_endpoint.signed_request);
}

void
account_service::download_id_trade(const types::download_id_request& request,
                                   callback_type<types::download_id_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(download_id_trade(request)); });
}

result<types::download_link_response>
account_service::download_link_trade(const types::download_link_request& request)
{
    return owner_.execute<types::download_link_response>(download_link_trade_endpoint.method,
                                                         std::string{ download_link_trade_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_trade_endpoint.signed_request);
}

void
account_service::download_link_trade(const types::download_link_request& request,
                                     callback_type<types::download_link_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(download_link_trade(request)); });
}

} // namespace binapi2::fapi::rest
