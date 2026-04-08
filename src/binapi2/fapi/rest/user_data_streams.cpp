// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the listen key lifecycle for user data streams. The Binance
/// API requires a listen key to establish a user data WebSocket connection:
///   start()     - POST to create a new listen key (valid for 60 minutes)
///   keepalive() - PUT to extend the key's validity (must be called < 60 min)
///   close()     - DELETE to invalidate the key and close the stream
/// None of these endpoints take query parameters; authentication is via the
/// X-MBX-APIKEY header (injected by the transport layer).

#include <binapi2/fapi/rest/user_data_streams.hpp>

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

user_data_stream_service::user_data_stream_service(binapi2::fapi::client& owner) noexcept : owner_(owner) {}

result<types::listen_key_response_t>
user_data_stream_service::start()
{
    return owner_.execute<types::listen_key_response_t>(start_listen_key_endpoint.method,
                                                      std::string{ start_listen_key_endpoint.path },
                                                      {},
                                                      start_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::listen_key_response_t>>
user_data_stream_service::async_start()
{
    co_return co_await owner_.async_execute<types::listen_key_response_t>(start_listen_key_endpoint.method,
                                                                        std::string{ start_listen_key_endpoint.path },
                                                                        {},
                                                                        start_listen_key_endpoint.signed_request);
}

result<types::listen_key_response_t>
user_data_stream_service::keepalive()
{
    return owner_.execute<types::listen_key_response_t>(keepalive_listen_key_endpoint.method,
                                                      std::string{ keepalive_listen_key_endpoint.path },
                                                      {},
                                                      keepalive_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::listen_key_response_t>>
user_data_stream_service::async_keepalive()
{
    co_return co_await owner_.async_execute<types::listen_key_response_t>(keepalive_listen_key_endpoint.method,
                                                                        std::string{ keepalive_listen_key_endpoint.path },
                                                                        {},
                                                                        keepalive_listen_key_endpoint.signed_request);
}

result<types::empty_response_t>
user_data_stream_service::close()
{
    return owner_.execute<types::empty_response_t>(close_listen_key_endpoint.method,
                                                 std::string{ close_listen_key_endpoint.path },
                                                 {},
                                                 close_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::empty_response_t>>
user_data_stream_service::async_close()
{
    co_return co_await owner_.async_execute<types::empty_response_t>(close_listen_key_endpoint.method,
                                                                   std::string{ close_listen_key_endpoint.path },
                                                                   {},
                                                                   close_listen_key_endpoint.signed_request);
}

} // namespace binapi2::fapi::rest
