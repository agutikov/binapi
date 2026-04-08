// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Account service for USD-M Futures authenticated account endpoints.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/cobalt/task.hpp>

#include <vector>

namespace binapi2::fapi::rest {

/// @brief Service group for authenticated account information and configuration endpoints.
///
/// Request types with a unique endpoint mapping (e.g. position_risk_request_t,
/// income_history_request_t) are exposed as using declarations and dispatched through
/// the inherited execute/async_execute template methods.
///
/// Named methods are provided for:
///   - **Parameterless endpoints**: account_information_t(), balances(), account_config(), etc.
///   - **Shared request types**: download_id_request_t and download_link_request_t are each
///     used by three endpoints (transaction, order, trade), so named methods disambiguate.
///
/// All endpoints in this service require HMAC signing (user_data security level).
class account_service : public service
{
public:
    using service::service;

    // Request types with 1:1 endpoint mapping (use execute/async_execute).
    using position_risk_request_t = types::position_risk_request_t;
    using symbol_config_request_t = types::symbol_config_request_t;
    using income_history_request_t = types::income_history_request_t;
    using leverage_bracket_request_t = types::leverage_bracket_request_t;
    using commission_rate_request_t = types::commission_rate_request_t;
    using toggle_bnb_burn_request_t = types::toggle_bnb_burn_request_t;
    using quantitative_rules_request_t = types::quantitative_rules_request_t;
    using pm_account_info_request_t = types::pm_account_info_request_t;

    /// @brief Fetch full account information (positions, balances, settings).
    [[nodiscard]] result<types::account_information_t> account_information_t();
    /// @brief Async variant of account_information_t.
    [[nodiscard]] boost::cobalt::task<result<types::account_information_t>> async_account_information();

    /// @brief Fetch all futures account balances.
    [[nodiscard]] result<std::vector<types::futures_account_balance_t>> balances();
    /// @brief Async variant of balances.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::futures_account_balance_t>>> async_balances();

    /// @brief Fetch account configuration (position mode, multi-asset mode, etc.).
    [[nodiscard]] result<types::account_config_response_t> account_config();
    /// @brief Async variant of account_config.
    [[nodiscard]] boost::cobalt::task<result<types::account_config_response_t>> async_account_config();

    /// @brief Query current multi-assets margin mode.
    [[nodiscard]] result<types::multi_assets_mode_response_t> get_multi_assets_mode();
    /// @brief Async variant of get_multi_assets_mode.
    [[nodiscard]] boost::cobalt::task<result<types::multi_assets_mode_response_t>> async_get_multi_assets_mode();

    /// @brief Query current position mode (hedge or one-way).
    [[nodiscard]] result<types::position_mode_response_t> get_position_mode();
    /// @brief Async variant of get_position_mode.
    [[nodiscard]] boost::cobalt::task<result<types::position_mode_response_t>> async_get_position_mode();

    /// @brief Fetch current order rate limit usage.
    [[nodiscard]] result<std::vector<types::rate_limit_t>> rate_limit_order();
    /// @brief Async variant of rate_limit_order.
    [[nodiscard]] boost::cobalt::task<result<std::vector<types::rate_limit_t>>> async_rate_limit_order();

    /// @brief Query whether BNB fee burn is enabled.
    [[nodiscard]] result<types::bnb_burn_status_response_t> get_bnb_burn();
    /// @brief Async variant of get_bnb_burn.
    [[nodiscard]] boost::cobalt::task<result<types::bnb_burn_status_response_t>> async_get_bnb_burn();

    /// @brief Request a download ID for transaction history export.
    /// @param request  Date range parameters for the export.
    [[nodiscard]] result<types::download_id_response_t> download_id_transaction(const types::download_id_request_t& request);
    /// @brief Async variant of download_id_transaction.
    [[nodiscard]] boost::cobalt::task<result<types::download_id_response_t>> async_download_id_transaction(const types::download_id_request_t& request);

    /// @brief Retrieve the download link for a previously requested transaction export.
    /// @param request  The download ID obtained from download_id_transaction.
    [[nodiscard]] result<types::download_link_response_t> download_link_transaction(const types::download_link_request_t& request);
    /// @brief Async variant of download_link_transaction.
    [[nodiscard]] boost::cobalt::task<result<types::download_link_response_t>> async_download_link_transaction(const types::download_link_request_t& request);

    /// @brief Request a download ID for order history export.
    /// @param request  Date range parameters.
    [[nodiscard]] result<types::download_id_response_t> download_id_order(const types::download_id_request_t& request);
    /// @brief Async variant of download_id_order.
    [[nodiscard]] boost::cobalt::task<result<types::download_id_response_t>> async_download_id_order(const types::download_id_request_t& request);

    /// @brief Retrieve the download link for a previously requested order export.
    /// @param request  The download ID obtained from download_id_order.
    [[nodiscard]] result<types::download_link_response_t> download_link_order(const types::download_link_request_t& request);
    /// @brief Async variant of download_link_order.
    [[nodiscard]] boost::cobalt::task<result<types::download_link_response_t>> async_download_link_order(const types::download_link_request_t& request);

    /// @brief Request a download ID for trade history export.
    /// @param request  Date range parameters.
    [[nodiscard]] result<types::download_id_response_t> download_id_trade(const types::download_id_request_t& request);
    /// @brief Async variant of download_id_trade.
    [[nodiscard]] boost::cobalt::task<result<types::download_id_response_t>> async_download_id_trade(const types::download_id_request_t& request);

    /// @brief Retrieve the download link for a previously requested trade export.
    /// @param request  The download ID obtained from download_id_trade.
    [[nodiscard]] result<types::download_link_response_t> download_link_trade(const types::download_link_request_t& request);
    /// @brief Async variant of download_link_trade.
    [[nodiscard]] boost::cobalt::task<result<types::download_link_response_t>> async_download_link_trade(const types::download_link_request_t& request);
};

} // namespace binapi2::fapi::rest
