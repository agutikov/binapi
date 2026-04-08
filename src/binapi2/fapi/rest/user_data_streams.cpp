// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the listen key lifecycle for user data streams.

#include <binapi2/fapi/rest/user_data_streams.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

user_data_stream_service::user_data_stream_service(pipeline& p) noexcept : pipeline_(p) {}

boost::cobalt::task<result<types::listen_key_response_t>>
user_data_stream_service::async_start()
{
    co_return co_await pipeline_.async_execute<types::listen_key_response_t>(start_listen_key_endpoint.method,
                                                                            std::string{ start_listen_key_endpoint.path },
                                                                            {},
                                                                            start_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::listen_key_response_t>>
user_data_stream_service::async_keepalive()
{
    co_return co_await pipeline_.async_execute<types::listen_key_response_t>(keepalive_listen_key_endpoint.method,
                                                                            std::string{ keepalive_listen_key_endpoint.path },
                                                                            {},
                                                                            keepalive_listen_key_endpoint.signed_request);
}

boost::cobalt::task<result<types::empty_response_t>>
user_data_stream_service::async_close()
{
    co_return co_await pipeline_.async_execute<types::empty_response_t>(close_listen_key_endpoint.method,
                                                                       std::string{ close_listen_key_endpoint.path },
                                                                       {},
                                                                       close_listen_key_endpoint.signed_request);
}

} // namespace binapi2::fapi::rest
