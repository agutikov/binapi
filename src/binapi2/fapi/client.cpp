// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the client facade. Construction initializes HTTP transport
/// and REST pipeline. WebSocket components are lazy-initialized on first access.

#include <binapi2/fapi/client.hpp>

namespace binapi2::fapi {

client::client(config cfg) :
    account(pipeline_), convert(pipeline_), market_data(pipeline_), trade(pipeline_), user_data_streams(pipeline_),
    cfg_(std::move(cfg)),
    http_(cfg_),
    pipeline_(cfg_, http_)
{
}

client::~client() = default;

config& client::configuration() noexcept { return cfg_; }
const config& client::configuration() const noexcept { return cfg_; }
rest::pipeline& client::rest() noexcept { return pipeline_; }

websocket_api::client&
client::ws_api()
{
    if (!ws_api_)
        ws_api_ = std::make_unique<websocket_api::client>(cfg_);
    return *ws_api_;
}

streams::market_streams&
client::streams()
{
    if (!streams_)
        streams_ = std::make_unique<streams::market_streams>(cfg_);
    return *streams_;
}

streams::user_streams&
client::user_streams()
{
    if (!user_streams_)
        user_streams_ = std::make_unique<streams::user_streams>(cfg_);
    return *user_streams_;
}

} // namespace binapi2::fapi
