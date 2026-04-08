// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the async user data stream client.

#include <binapi2/fapi/streams/user_streams.hpp>

namespace binapi2::fapi::streams {

user_streams::user_streams(config cfg) :
    transport_(cfg), cfg_(std::move(cfg))
{
}

boost::cobalt::task<result<void>>
user_streams::async_connect(std::string listen_key)
{
    const auto target = cfg_.stream_base_target + "/" + listen_key;
    co_return co_await transport_.async_connect(cfg_.stream_host, cfg_.stream_port, target);
}

boost::cobalt::task<result<std::string>>
user_streams::async_read_text()
{
    co_return co_await transport_.async_read_text();
}

boost::cobalt::task<result<void>>
user_streams::async_close()
{
    co_return co_await transport_.async_close();
}

} // namespace binapi2::fapi::streams
