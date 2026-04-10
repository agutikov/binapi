// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Account REST service — signed endpoints for balances, positions, config.

#pragma once

#include <binapi2/fapi/rest/services/service.hpp>
#include <binapi2/fapi/types/account.hpp>

namespace binapi2::fapi::rest {

class account_service : public service
{
public:
    using service::service;

    template<class Request>
        requires is_account_request<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>
    {
        co_return co_await pipeline_.async_execute(request);
    }

    using account_information_request = types::account_information_request_t;
    using balances_request = types::balances_request_t;
    using account_config_request = types::account_config_request_t;
    using position_risk_request = types::position_risk_request_t;
    using symbol_config_request = types::symbol_config_request_t;
    using income_history_request = types::income_history_request_t;
    using leverage_bracket_request = types::leverage_bracket_request_t;
    using commission_rate_request = types::commission_rate_request_t;
    using get_multi_assets_mode_request = types::get_multi_assets_mode_request_t;
    using get_position_mode_request = types::get_position_mode_request_t;
    using rate_limit_order_request = types::rate_limit_order_request_t;
    using get_bnb_burn_request = types::get_bnb_burn_request_t;
    using toggle_bnb_burn_request = types::toggle_bnb_burn_request_t;
    using quantitative_rules_request = types::quantitative_rules_request_t;
    using pm_account_info_request = types::pm_account_info_request_t;
    using download_id_transaction_request = types::download_id_transaction_request_t;
    using download_link_transaction_request = types::download_link_transaction_request_t;
    using download_id_order_request = types::download_id_order_request_t;
    using download_link_order_request = types::download_link_order_request_t;
    using download_id_trade_request = types::download_id_trade_request_t;
    using download_link_trade_request = types::download_link_trade_request_t;
};

} // namespace binapi2::fapi::rest
