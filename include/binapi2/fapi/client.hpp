// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Top-level USD-M Futures API client facade.
///
/// Pure container for config, transport, REST pipeline, services, and
/// lazy WebSocket components. Does not own an executor.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/rest/account.hpp>
#include <binapi2/fapi/rest/convert.hpp>
#include <binapi2/fapi/rest/market_data.hpp>
#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/rest/trade.hpp>
#include <binapi2/fapi/rest/user_data_streams.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/streams/user_streams.hpp>
#include <binapi2/fapi/transport/http_client.hpp>
#include <binapi2/fapi/websocket_api/client.hpp>

#include <memory>

namespace binapi2::fapi {

/// @brief USD-M Futures API client facade.
///
/// Container for configuration, HTTP transport, REST pipeline, REST services,
/// and lazy-initialized WebSocket components. All API methods are async
/// (cobalt::task). The client does not own an executor — coroutines run on
/// whatever executor drives them.
class client
{
public:
    explicit client(config cfg);
    ~client();

    [[nodiscard]] config& configuration() noexcept;
    [[nodiscard]] const config& configuration() const noexcept;

    [[nodiscard]] rest::pipeline& rest() noexcept;

    [[nodiscard]] websocket_api::client& ws_api();
    [[nodiscard]] streams::market_streams& streams();
    [[nodiscard]] streams::user_streams& user_streams();

    rest::account_service account;
    rest::convert_service convert;
    rest::market_data_service market_data;
    rest::trade_service trade;
    rest::user_data_stream_service user_data_streams;

private:
    config cfg_;
    transport::http_client http_;
    rest::pipeline pipeline_;

    std::unique_ptr<websocket_api::client> ws_api_;
    std::unique_ptr<streams::market_streams> streams_;
    std::unique_ptr<streams::user_streams> user_streams_;
};

} // namespace binapi2::fapi
