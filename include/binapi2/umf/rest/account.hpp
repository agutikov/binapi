#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/account.hpp>

#include <vector>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

class account_service {
  public:
    explicit account_service(client &owner) noexcept;

    [[nodiscard]] result<types::account_information> account_information();
    [[nodiscard]] result<std::vector<types::futures_account_balance>> balances();
    [[nodiscard]] result<std::vector<types::position_risk>> position_risk(const types::position_risk_request &request = {});

  private:
    client &owner_;
};

} // namespace binapi2::umf::rest
