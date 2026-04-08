// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Top-level USD-M Futures API client facade.
///
/// Container for REST services, WebSocket API, and stream clients.
/// Does not own an executor — async methods return cobalt::task that
/// runs on whatever executor drives them.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
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
#include <stdexcept>

namespace binapi2::fapi {

/// @brief Tag type selecting async-only mode (no background io_thread).
struct async_mode_t { explicit async_mode_t() = default; };

/// @brief Tag constant for constructing a client in async-only mode.
inline constexpr async_mode_t async_mode{};

/// @brief USD-M Futures API client facade.
///
/// Container for configuration, HTTP transport, REST pipeline, REST services,
/// and lazy-initialized WebSocket components.
///
/// Two construction modes:
///   - **Sync + async** (default): creates a background io_thread.
///   - **Async-only** (with async_mode): no io_thread. Only co_await works.
class client
{
public:
    /// @brief Construct with sync + async support (creates background io_thread).
    explicit client(config cfg);

    /// @brief Construct in async-only mode (no background thread).
    client(config cfg, async_mode_t);

    ~client();

    [[nodiscard]] config& configuration() noexcept;
    [[nodiscard]] const config& configuration() const noexcept;

    /// @brief Access the REST pipeline for low-level async_execute calls.
    [[nodiscard]] rest::pipeline& rest() noexcept;

    /// @brief Whether this client has an io_thread (sync capable).
    [[nodiscard]] bool has_io_thread() const noexcept;

    /// @brief Spawn a cobalt task on the io_thread and block until completion.
    /// @throws std::logic_error if called in async-only mode.
    template<typename T>
    T run_sync(boost::cobalt::task<T> task)
    {
        if (!io_thread_) throw std::logic_error("run_sync requires io_thread (not async_mode)");
        return io_thread_->run_sync(std::move(task));
    }

    /// @brief Access the WebSocket API client (lazy-initialized).
    [[nodiscard]] websocket_api::client& ws_api();

    /// @brief Access the market data streams client (lazy-initialized).
    [[nodiscard]] streams::market_streams& streams();

    /// @brief Access the user data streams client (lazy-initialized).
    [[nodiscard]] streams::user_streams& user_streams();

    // REST service groups — delegate to pipeline via rest::service base.
    rest::account_service account;
    rest::convert_service convert;
    rest::market_data_service market_data;
    rest::trade_service trade;
    rest::user_data_stream_service user_data_streams;

private:
    std::unique_ptr<detail::io_thread> io_thread_;  // nullptr in async-only mode
    config cfg_;
    transport::http_client http_;
    rest::pipeline pipeline_;

    // Lazy WebSocket components.
    std::unique_ptr<websocket_api::client> ws_api_;
    std::unique_ptr<streams::market_streams> streams_;
    std::unique_ptr<streams::user_streams> user_streams_;
};

} // namespace binapi2::fapi
