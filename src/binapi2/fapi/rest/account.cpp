// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/account.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <binapi2/fapi/query.hpp>

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
    return owner_.execute<std::vector<types::position_risk>>(position_risk_endpoint.method,
                                                             std::string{ position_risk_endpoint.path },
                                                             to_query_map(request),
                                                             position_risk_endpoint.signed_request);
}

void
account_service::position_risk(const types::position_risk_request& request,
                               callback_type<std::vector<types::position_risk>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(position_risk(request)); });
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

result<std::vector<types::symbol_config_entry>>
account_service::symbol_config(const types::symbol_config_request& request)
{
    return owner_.execute<std::vector<types::symbol_config_entry>>(symbol_config_endpoint.method,
                                                                    std::string{ symbol_config_endpoint.path },
                                                                    to_query_map(request),
                                                                    symbol_config_endpoint.signed_request);
}

void
account_service::symbol_config(const types::symbol_config_request& request,
                               callback_type<std::vector<types::symbol_config_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(symbol_config(request)); });
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

result<std::vector<types::income_history_entry>>
account_service::income_history(const types::income_history_request& request)
{
    return owner_.execute<std::vector<types::income_history_entry>>(income_history_endpoint.method,
                                                                    std::string{ income_history_endpoint.path },
                                                                    to_query_map(request),
                                                                    income_history_endpoint.signed_request);
}

void
account_service::income_history(const types::income_history_request& request,
                                callback_type<std::vector<types::income_history_entry>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(income_history(request)); });
}

result<std::vector<types::symbol_leverage_brackets>>
account_service::leverage_brackets(const types::leverage_bracket_request& request)
{
    return owner_.execute<std::vector<types::symbol_leverage_brackets>>(leverage_brackets_endpoint.method,
                                                                        std::string{ leverage_brackets_endpoint.path },
                                                                        to_query_map(request),
                                                                        leverage_brackets_endpoint.signed_request);
}

void
account_service::leverage_brackets(const types::leverage_bracket_request& request,
                                   callback_type<std::vector<types::symbol_leverage_brackets>> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(leverage_brackets(request)); });
}

result<types::commission_rate_response>
account_service::commission_rate(const types::commission_rate_request& request)
{
    return owner_.execute<types::commission_rate_response>(commission_rate_endpoint.method,
                                                           std::string{ commission_rate_endpoint.path },
                                                           to_query_map(request),
                                                           commission_rate_endpoint.signed_request);
}

void
account_service::commission_rate(const types::commission_rate_request& request,
                                 callback_type<types::commission_rate_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(commission_rate(request)); });
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

result<types::bnb_burn_status_response>
account_service::toggle_bnb_burn(const types::toggle_bnb_burn_request& request)
{
    return owner_.execute<types::bnb_burn_status_response>(toggle_bnb_burn_endpoint.method,
                                                           std::string{ toggle_bnb_burn_endpoint.path },
                                                           to_query_map(request),
                                                           toggle_bnb_burn_endpoint.signed_request);
}

void
account_service::toggle_bnb_burn(const types::toggle_bnb_burn_request& request,
                                 callback_type<types::bnb_burn_status_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(toggle_bnb_burn(request)); });
}

result<types::quantitative_rules_response>
account_service::quantitative_rules(const types::quantitative_rules_request& request)
{
    return owner_.execute<types::quantitative_rules_response>(quantitative_rules_endpoint.method,
                                                              std::string{ quantitative_rules_endpoint.path },
                                                              to_query_map(request),
                                                              quantitative_rules_endpoint.signed_request);
}

void
account_service::quantitative_rules(const types::quantitative_rules_request& request,
                                    callback_type<types::quantitative_rules_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(quantitative_rules(request)); });
}

result<types::pm_account_info_response>
account_service::pm_account_info(const types::pm_account_info_request& request)
{
    return owner_.execute<types::pm_account_info_response>(pm_account_info_endpoint.method,
                                                           std::string{ pm_account_info_endpoint.path },
                                                           to_query_map(request),
                                                           pm_account_info_endpoint.signed_request);
}

void
account_service::pm_account_info(const types::pm_account_info_request& request,
                                 callback_type<types::pm_account_info_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(pm_account_info(request)); });
}

} // namespace binapi2::fapi::rest
