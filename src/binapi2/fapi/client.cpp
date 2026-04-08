// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the top-level client facade. Construction initializes the
/// HTTP transport and REST pipeline. WebSocket components are lazy-initialized
/// on first access.

#include <binapi2/fapi/client.hpp>

namespace binapi2::fapi {

client::client(config cfg) :
    account(pipeline_), convert(pipeline_), market_data(pipeline_), trade(pipeline_), user_data_streams(pipeline_),
    io_thread_(std::make_unique<detail::io_thread>()),
    cfg_(std::move(cfg)),
    http_(*io_thread_, cfg_),
    pipeline_(cfg_, http_)
{
}

client::client(config cfg, async_mode_t) :
    account(pipeline_), convert(pipeline_), market_data(pipeline_), trade(pipeline_), user_data_streams(pipeline_),
    cfg_(std::move(cfg)),
    http_(cfg_),
    pipeline_(cfg_, http_)
{
}

client::~client() = default;

config&
client::configuration() noexcept
{
    return cfg_;
}

const config&
client::configuration() const noexcept
{
    return cfg_;
}

rest::pipeline&
client::rest() noexcept
{
    return pipeline_;
}

bool
client::has_io_thread() const noexcept
{
    return io_thread_ != nullptr;
}

websocket_api::client&
client::ws_api()
{
    if (!ws_api_) {
        if (io_thread_)
            ws_api_ = std::make_unique<websocket_api::client>(*io_thread_, cfg_);
        else
            ws_api_ = std::make_unique<websocket_api::client>(cfg_);
    }
    return *ws_api_;
}

streams::market_streams&
client::streams()
{
    if (!streams_) {
        if (io_thread_)
            streams_ = std::make_unique<streams::market_streams>(*io_thread_, cfg_);
        else
            streams_ = std::make_unique<streams::market_streams>(cfg_);
    }
    return *streams_;
}

streams::user_streams&
client::user_streams()
{
    if (!user_streams_) {
        if (io_thread_)
            user_streams_ = std::make_unique<streams::user_streams>(*io_thread_, cfg_);
        else
            user_streams_ = std::make_unique<streams::user_streams>(cfg_);
    }
    return *user_streams_;
}

} // namespace binapi2::fapi
