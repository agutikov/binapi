// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file client.hpp
/// @brief Self-contained REST client owning a persistent HTTP connection,
///        request pipeline, and all REST service groups.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/rest/services/account.hpp>
#include <binapi2/fapi/rest/services/convert.hpp>
#include <binapi2/fapi/rest/services/market_data.hpp>
#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/rest/services/trade.hpp>
#include <binapi2/fapi/rest/services/user_data_streams.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/http_client.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

/// @brief Self-contained REST API client.
///
/// Owns a persistent HTTP connection, the request pipeline, and all REST
/// service groups. Each client represents one connection to the REST
/// endpoint (rest_host:rest_port).
///
/// Non-copyable, non-movable. Created via futures_usdm_api::create_rest_client()
/// which returns a connected unique_ptr, or constructed directly:
///
///   auto rest = std::make_unique<rest::client>(cfg);
///   co_await rest->async_connect();
class client
{
    // Private members declared first — initialized before public services.
    config cfg_;
    transport::http_client http_;
    pipeline pipeline_;

public:
    explicit client(config cfg);

    client(const client&) = delete;
    client& operator=(const client&) = delete;

    /// @brief Establish the HTTP connection (DNS + TCP + TLS).
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect();

    [[nodiscard]] config& configuration() noexcept { return cfg_; }
    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }
    [[nodiscard]] pipeline& rest_pipeline() noexcept { return pipeline_; }

    account_service account;
    convert_service convert;
    market_data_service market_data;
    trade_service trade;
    user_data_stream_service user_data_streams;
};

} // namespace binapi2::fapi::rest
