#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/trade.hpp>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

class trade_service {
  public:
    explicit trade_service(client &owner) noexcept;

    [[nodiscard]] result<types::order_response> new_order(const types::new_order_request &request);
    [[nodiscard]] result<types::order_response> modify_order(const types::modify_order_request &request);
    [[nodiscard]] result<types::order_response> cancel_order(const types::cancel_order_request &request);
    [[nodiscard]] result<types::order_response> query_order(const types::query_order_request &request);

  private:
    client &owner_;
};

} // namespace binapi2::umf::rest
