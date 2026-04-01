// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/cobalt/task.hpp>

#include <vector>

namespace binapi2::fapi::rest {

class account_service : public service
{
public:
    using service::service;

    // Request types with 1:1 endpoint mapping (use execute/async_execute).
    using position_risk_request = types::position_risk_request;
    using symbol_config_request = types::symbol_config_request;
    using income_history_request = types::income_history_request;
    using leverage_bracket_request = types::leverage_bracket_request;
    using commission_rate_request = types::commission_rate_request;
    using toggle_bnb_burn_request = types::toggle_bnb_burn_request;
    using quantitative_rules_request = types::quantitative_rules_request;
    using pm_account_info_request = types::pm_account_info_request;

    // Parameterless endpoints.
    [[nodiscard]] result<types::account_information> account_information();
    [[nodiscard]] boost::cobalt::task<result<types::account_information>> async_account_information();
    [[nodiscard]] result<std::vector<types::futures_account_balance>> balances();
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::futures_account_balance>>> async_balances();
    [[nodiscard]] result<types::account_config_response> account_config();
    [[nodiscard]] boost::cobalt::task<result<types::account_config_response>> async_account_config();
    [[nodiscard]] result<types::multi_assets_mode_response> get_multi_assets_mode();
    [[nodiscard]] boost::cobalt::task<result<types::multi_assets_mode_response>> async_get_multi_assets_mode();
    [[nodiscard]] result<types::position_mode_response> get_position_mode();
    [[nodiscard]] boost::cobalt::task<result<types::position_mode_response>> async_get_position_mode();
    [[nodiscard]] result<std::vector<types::rate_limit>> rate_limit_order();
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::rate_limit>>> async_rate_limit_order();
    [[nodiscard]] result<types::bnb_burn_status_response> get_bnb_burn();
    [[nodiscard]] boost::cobalt::task<result<types::bnb_burn_status_response>> async_get_bnb_burn();

    // Methods for shared request types (download_id_request/download_link_request
    // used by 3 endpoints each).
    [[nodiscard]] result<types::download_id_response> download_id_transaction(const types::download_id_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::download_id_response>> async_download_id_transaction(const types::download_id_request& request);
    [[nodiscard]] result<types::download_link_response> download_link_transaction(const types::download_link_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::download_link_response>> async_download_link_transaction(const types::download_link_request& request);
    [[nodiscard]] result<types::download_id_response> download_id_order(const types::download_id_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::download_id_response>> async_download_id_order(const types::download_id_request& request);
    [[nodiscard]] result<types::download_link_response> download_link_order(const types::download_link_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::download_link_response>> async_download_link_order(const types::download_link_request& request);
    [[nodiscard]] result<types::download_id_response> download_id_trade(const types::download_id_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::download_id_response>> async_download_id_trade(const types::download_id_request& request);
    [[nodiscard]] result<types::download_link_response> download_link_trade(const types::download_link_request& request);
    [[nodiscard]] boost::cobalt::task<result<types::download_link_response>> async_download_link_trade(const types::download_link_request& request);
};

} // namespace binapi2::fapi::rest
