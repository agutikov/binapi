// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the futures_usdm_api async factory.

#include <binapi2/futures_usdm_api.hpp>

namespace binapi2 {

futures_usdm_api::futures_usdm_api(fapi::config cfg) :
    cfg_(std::move(cfg))
{
}

fapi::config& futures_usdm_api::configuration() noexcept { return cfg_; }
const fapi::config& futures_usdm_api::configuration() const noexcept { return cfg_; }

boost::cobalt::task<fapi::result<std::unique_ptr<fapi::rest::client>>>
futures_usdm_api::create_rest_client()
{
    auto c = std::make_unique<fapi::rest::client>(cfg_);
    auto conn = co_await c->async_connect();
    if (!conn)
        co_return fapi::result<std::unique_ptr<fapi::rest::client>>::failure(conn.err);
    co_return fapi::result<std::unique_ptr<fapi::rest::client>>::success(std::move(c));
}

boost::cobalt::task<fapi::result<std::unique_ptr<fapi::websocket_api::client>>>
futures_usdm_api::create_ws_api_client()
{
    auto c = std::make_unique<fapi::websocket_api::client>(cfg_);
    auto conn = co_await c->async_connect();
    if (!conn)
        co_return fapi::result<std::unique_ptr<fapi::websocket_api::client>>::failure(conn.err);
    co_return fapi::result<std::unique_ptr<fapi::websocket_api::client>>::success(std::move(c));
}

std::unique_ptr<fapi::streams::market_stream>
futures_usdm_api::create_market_stream()
{
    return std::make_unique<fapi::streams::market_stream>(cfg_);
}

std::unique_ptr<fapi::streams::combined_market_stream>
futures_usdm_api::create_combined_market_stream()
{
    return std::make_unique<fapi::streams::combined_market_stream>(cfg_);
}

std::unique_ptr<fapi::streams::dynamic_market_stream>
futures_usdm_api::create_dynamic_market_stream()
{
    return std::make_unique<fapi::streams::dynamic_market_stream>(cfg_);
}

std::unique_ptr<fapi::streams::user_stream>
futures_usdm_api::create_user_stream()
{
    return std::make_unique<fapi::streams::user_stream>(cfg_);
}

} // namespace binapi2
