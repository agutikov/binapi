#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/common.hpp>

#include <functional>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

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

} // namespace binapi2::umf::rest
