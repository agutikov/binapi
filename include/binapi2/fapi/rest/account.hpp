// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/account.hpp>

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

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
