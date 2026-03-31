#include <binapi2/umf/rest/user_data_streams.hpp>

#include <binapi2/umf/rest/generated_endpoints.hpp>

#include "common.hpp"

namespace binapi2::umf::rest {

user_data_stream_service::user_data_stream_service(binapi2::umf::client& owner) noexcept : owner_(owner) {}

result<types::listen_key_response>
user_data_stream_service::start()
{
    return owner_.execute<types::listen_key_response>(start_listen_key_endpoint.method,
                                                      std::string{ start_listen_key_endpoint.path },
                                                      {},
                                                      start_listen_key_endpoint.signed_request);
}

void
user_data_stream_service::start(callback_type<types::listen_key_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(start()); });
}

result<types::listen_key_response>
user_data_stream_service::keepalive()
{
    return owner_.execute<types::listen_key_response>(keepalive_listen_key_endpoint.method,
                                                      std::string{ keepalive_listen_key_endpoint.path },
                                                      {},
                                                      keepalive_listen_key_endpoint.signed_request);
}

void
user_data_stream_service::keepalive(callback_type<types::listen_key_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(keepalive()); });
}

result<types::empty_response>
user_data_stream_service::close()
{
    return owner_.execute<types::empty_response>(close_listen_key_endpoint.method,
                                                 std::string{ close_listen_key_endpoint.path },
                                                 {},
                                                 close_listen_key_endpoint.signed_request);
}

void
user_data_stream_service::close(callback_type<types::empty_response> callback)
{
    detail::post_callback(owner_.context(), [this, callback = std::move(callback)]() mutable { callback(close()); });
}

} // namespace binapi2::umf::rest
