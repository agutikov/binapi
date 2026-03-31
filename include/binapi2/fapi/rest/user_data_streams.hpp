// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <functional>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class user_data_stream_service
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    explicit user_data_stream_service(client& owner) noexcept;

    [[nodiscard]] result<types::listen_key_response> start();
    void start(callback_type<types::listen_key_response> callback);
    [[nodiscard]] result<types::listen_key_response> keepalive();
    void keepalive(callback_type<types::listen_key_response> callback);
    [[nodiscard]] result<types::empty_response> close();
    void close(callback_type<types::empty_response> callback);

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
