#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/trade.hpp>

#include <functional>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

class trade_service
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    explicit trade_service(client& owner) noexcept;

    [[nodiscard]] result<types::order_response> new_order(const types::new_order_request& request);
    void new_order(const types::new_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> modify_order(const types::modify_order_request& request);
    void modify_order(const types::modify_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> cancel_order(const types::cancel_order_request& request);
    void cancel_order(const types::cancel_order_request& request, callback_type<types::order_response> callback);
    [[nodiscard]] result<types::order_response> query_order(const types::query_order_request& request);
    void query_order(const types::query_order_request& request, callback_type<types::order_response> callback);

private:
    client& owner_;
};

} // namespace binapi2::umf::rest
