// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <functional>
#include <vector>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class account_service
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    explicit account_service(client& owner) noexcept;

    [[nodiscard]] result<types::account_information> account_information();
    void account_information(callback_type<types::account_information> callback);
    [[nodiscard]] result<std::vector<types::futures_account_balance>> balances();
    void balances(callback_type<std::vector<types::futures_account_balance>> callback);
    [[nodiscard]] result<std::vector<types::position_risk>> position_risk(const types::position_risk_request& request = {});
    void position_risk(const types::position_risk_request& request, callback_type<std::vector<types::position_risk>> callback);
    [[nodiscard]] result<types::account_config_response> account_config();
    void account_config(callback_type<types::account_config_response> callback);
    [[nodiscard]] result<std::vector<types::symbol_config_entry>> symbol_config(const types::symbol_config_request& request = {});
    void symbol_config(const types::symbol_config_request& request, callback_type<std::vector<types::symbol_config_entry>> callback);
    [[nodiscard]] result<types::multi_assets_mode_response> get_multi_assets_mode();
    void get_multi_assets_mode(callback_type<types::multi_assets_mode_response> callback);
    [[nodiscard]] result<types::position_mode_response> get_position_mode();
    void get_position_mode(callback_type<types::position_mode_response> callback);
    [[nodiscard]] result<std::vector<types::income_history_entry>> income_history(const types::income_history_request& request = {});
    void income_history(const types::income_history_request& request, callback_type<std::vector<types::income_history_entry>> callback);
    [[nodiscard]] result<std::vector<types::symbol_leverage_brackets>> leverage_brackets(const types::leverage_bracket_request& request = {});
    void leverage_brackets(const types::leverage_bracket_request& request, callback_type<std::vector<types::symbol_leverage_brackets>> callback);
    [[nodiscard]] result<types::commission_rate_response> commission_rate(const types::commission_rate_request& request);
    void commission_rate(const types::commission_rate_request& request, callback_type<types::commission_rate_response> callback);
    [[nodiscard]] result<std::vector<types::rate_limit>> rate_limit_order();
    void rate_limit_order(callback_type<std::vector<types::rate_limit>> callback);
    [[nodiscard]] result<types::download_id_response> download_id_transaction(const types::download_id_request& request);
    void download_id_transaction(const types::download_id_request& request, callback_type<types::download_id_response> callback);
    [[nodiscard]] result<types::download_link_response> download_link_transaction(const types::download_link_request& request);
    void download_link_transaction(const types::download_link_request& request, callback_type<types::download_link_response> callback);
    [[nodiscard]] result<types::download_id_response> download_id_order(const types::download_id_request& request);
    void download_id_order(const types::download_id_request& request, callback_type<types::download_id_response> callback);
    [[nodiscard]] result<types::download_link_response> download_link_order(const types::download_link_request& request);
    void download_link_order(const types::download_link_request& request, callback_type<types::download_link_response> callback);
    [[nodiscard]] result<types::download_id_response> download_id_trade(const types::download_id_request& request);
    void download_id_trade(const types::download_id_request& request, callback_type<types::download_id_response> callback);
    [[nodiscard]] result<types::download_link_response> download_link_trade(const types::download_link_request& request);
    void download_link_trade(const types::download_link_request& request, callback_type<types::download_link_response> callback);
    [[nodiscard]] result<types::bnb_burn_status_response> get_bnb_burn();
    void get_bnb_burn(callback_type<types::bnb_burn_status_response> callback);
    [[nodiscard]] result<types::bnb_burn_status_response> toggle_bnb_burn(const types::toggle_bnb_burn_request& request);
    void toggle_bnb_burn(const types::toggle_bnb_burn_request& request, callback_type<types::bnb_burn_status_response> callback);
    [[nodiscard]] result<types::quantitative_rules_response> quantitative_rules(const types::quantitative_rules_request& request = {});
    void quantitative_rules(const types::quantitative_rules_request& request, callback_type<types::quantitative_rules_response> callback);

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
