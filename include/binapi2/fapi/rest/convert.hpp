// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/convert.hpp>
#include <functional>

namespace binapi2::fapi { class client; }

namespace binapi2::fapi::rest {

class convert_service {
public:
    template<typename T> using callback_type = std::function<void(result<T>)>;
    explicit convert_service(client& owner) noexcept;

    [[nodiscard]] result<types::convert_quote_response> get_quote(const types::convert_quote_request& request);
    void get_quote(const types::convert_quote_request& request, callback_type<types::convert_quote_response> callback);
    [[nodiscard]] result<types::convert_accept_response> accept_quote(const types::convert_accept_request& request);
    void accept_quote(const types::convert_accept_request& request, callback_type<types::convert_accept_response> callback);
    [[nodiscard]] result<types::convert_order_status_response> order_status(const types::convert_order_status_request& request);
    void order_status(const types::convert_order_status_request& request, callback_type<types::convert_order_status_response> callback);

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
