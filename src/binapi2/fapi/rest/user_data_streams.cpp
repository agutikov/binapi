// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/user_data_streams.hpp>

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

user_data_stream_service::user_data_stream_service(binapi2::fapi::client& owner) noexcept : owner_(owner) {}

result<types::listen_key_response>
user_data_stream_service::start()
{
    return owner_.execute<types::listen_key_response>(start_listen_key_endpoint.method,
                                                      std::string{ start_listen_key_endpoint.path },
                                                      {},
                                                      start_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::listen_key_response>>
user_data_stream_service::async_start()
{
    co_return start();
}

result<types::listen_key_response>
user_data_stream_service::keepalive()
{
    return owner_.execute<types::listen_key_response>(keepalive_listen_key_endpoint.method,
                                                      std::string{ keepalive_listen_key_endpoint.path },
                                                      {},
                                                      keepalive_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::listen_key_response>>
user_data_stream_service::async_keepalive()
{
    co_return keepalive();
}

result<types::empty_response>
user_data_stream_service::close()
{
    return owner_.execute<types::empty_response>(close_listen_key_endpoint.method,
                                                 std::string{ close_listen_key_endpoint.path },
                                                 {},
                                                 close_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::empty_response>>
user_data_stream_service::async_close()
{
    co_return close();
}

} // namespace binapi2::fapi::rest
