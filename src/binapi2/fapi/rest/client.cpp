// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the self-contained REST client.

#include <binapi2/fapi/rest/client.hpp>

namespace binapi2::fapi::rest {

client::client(config cfg) :
    cfg_(std::move(cfg)),
    http_(cfg_),
    pipeline_(cfg_, http_),
    account(pipeline_),
    convert(pipeline_),
    market_data(pipeline_),
    trade(pipeline_),
    user_data_streams(pipeline_)
{
}

boost::cobalt::task<result<void>>
client::async_connect()
{
    co_return co_await http_.async_connect();
}

} // namespace binapi2::fapi::rest
