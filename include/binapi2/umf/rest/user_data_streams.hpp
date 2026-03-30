#pragma once

#include <binapi2/umf/result.hpp>
#include <binapi2/umf/types/common.hpp>

namespace binapi2::umf {
class client;
}

namespace binapi2::umf::rest {

class user_data_stream_service
{
public:
    explicit user_data_stream_service(client& owner) noexcept;

    [[nodiscard]] result<types::listen_key_response> start();
    [[nodiscard]] result<types::listen_key_response> keepalive();
    [[nodiscard]] result<types::empty_response> close();

private:
    client& owner_;
};

} // namespace binapi2::umf::rest
