// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file market_stream.hpp
/// @brief Async market data stream client for Binance USD-M Futures WebSocket streams.
///
/// Thin facade over stream_connection that adds typed event parsing via
/// stream_traits and cobalt generators.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/stream_connection.hpp>
#include <binapi2/fapi/streams/stream_traits.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>
#include <glaze/glaze.hpp>

#include <string>
#include <utility>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Typed async event generator returned by market_stream::subscribe().
template<class Event>
using event_generator = boost::cobalt::generator<result<Event>>;

/// @brief Async client for Binance USD-M Futures market data WebSocket streams.
///
/// Wraps a stream_connection and adds typed event parsing. Usage:
///
///   auto stream = streams.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});
///   while (stream) {
///       auto event = co_await stream;
///       if (!event) break;
///       std::cout << event->best_bid_price << "\n";
///   }
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client>
class basic_market_stream
{
public:
    explicit basic_market_stream(config cfg) :
        conn_(std::move(cfg))
    {
    }

    basic_market_stream(const basic_market_stream&) = delete;
    basic_market_stream& operator=(const basic_market_stream&) = delete;

    // -- Generator --

    /// @brief Subscribe and return a typed async generator.
    template<class Subscription>
        requires has_stream_traits<Subscription>
    event_generator<typename stream_traits<Subscription>::event_type>
    subscribe(const Subscription& sub)
    {
        using Event = typename stream_traits<Subscription>::event_type;
        auto conn = co_await conn_.async_connect(
            conn_.configuration().stream_host,
            conn_.configuration().stream_port,
            stream_traits<Subscription>::target(conn_.configuration(), sub));
        if (!conn) {
            co_yield result<Event>::failure(conn.err);
        } else {
            bool running = true;
            while (running) {
                auto msg = co_await conn_.async_read_text();
                if (!msg) {
                    co_yield result<Event>::failure(msg.err);
                    running = false;
                } else {
                    Event event{};
                    glz::context ctx{};
                    if (auto ec = glz::read<fapi::detail::json_read_opts>(event, *msg, ctx)) {
                        co_yield result<Event>::failure({error_code::json, 0, 0, glz::format_error(ec, *msg), *msg});
                        running = false;
                    } else {
                        co_yield result<Event>::success(std::move(event));
                    }
                }
            }
        }
    }

    // -- Typed read --

    // -- Accessors --

    [[nodiscard]] Transport& transport() noexcept { return conn_.transport(); }
    [[nodiscard]] basic_stream_connection<Transport>& connection() noexcept { return conn_; }

private:
    basic_stream_connection<Transport> conn_;
};

/// @brief Default market streams using the WebSocket transport.
using market_stream = basic_market_stream<>;

} // namespace binapi2::fapi::streams
