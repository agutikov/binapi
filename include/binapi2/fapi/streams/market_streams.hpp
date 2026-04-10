// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file market_streams.hpp
/// @brief Async market data stream client for Binance USD-M Futures WebSocket streams.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/result.hpp>
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

/// @brief Typed async event generator returned by market_streams::subscribe().
template<class Event>
using event_generator = boost::cobalt::generator<result<Event>>;

namespace detail {

struct stream_control_request
{
    std::string method{};
    std::vector<std::string> params{};
    unsigned int id{};
};

struct stream_list_response
{
    std::vector<std::string> result{};
    unsigned int id{};
};

} // namespace detail

} // namespace binapi2::fapi::streams

template<>
struct glz::meta<binapi2::fapi::streams::detail::stream_control_request>
{
    using T = binapi2::fapi::streams::detail::stream_control_request;
    static constexpr auto value = object("method", &T::method, "params", &T::params, "id", &T::id);
};

template<>
struct glz::meta<binapi2::fapi::streams::detail::stream_list_response>
{
    using T = binapi2::fapi::streams::detail::stream_list_response;
    static constexpr auto value = object("result", &T::result, "id", &T::id);
};

namespace binapi2::fapi::streams {

/// @brief Async client for Binance USD-M Futures market data WebSocket streams.
///
/// Three usage styles:
///
/// Generator (recommended):
///   auto stream = streams.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});
///   while (stream) {
///       auto event = co_await stream;
///       if (!event) break;
///       std::cout << event->best_bid_price << "\n";
///   }
///
/// Typed connect + read:
///   co_await streams.async_connect(types::book_ticker_subscription{.symbol = "BTCUSDT"});
///   auto event = co_await streams.async_read_event<types::book_ticker_stream_event_t>();
///
/// Low-level (raw):
///   co_await streams.async_connect("/ws/btcusdt@bookTicker");
///   auto msg = co_await streams.async_read_text();
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client>
class basic_market_streams
{
public:
    explicit basic_market_streams(config cfg) :
        transport_(cfg), cfg_(std::move(cfg))
    {
    }

    // -- Generator --

    /// @brief Subscribe and return a typed async generator.
    ///
    /// Connects to the stream derived from stream_traits, then yields parsed
    /// events. The generator runs until an error occurs or the caller stops
    /// consuming. Destroying the generator does NOT close the connection —
    /// call async_close() explicitly if needed.
    template<class Subscription>
        requires has_stream_traits<Subscription>
    event_generator<typename stream_traits<Subscription>::event_type>
    subscribe(const Subscription& sub)
    {
        using Event = typename stream_traits<Subscription>::event_type;
        auto conn = co_await async_connect(stream_traits<Subscription>::target(cfg_, sub));
        if (!conn) {
            co_yield result<Event>::failure(conn.err);
        } else {
            bool running = true;
            while (running) {
                auto msg = co_await transport_.async_read_text();
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

    // -- Typed connect + read --

    /// @brief Connect using a subscription type with stream_traits.
    template<class Subscription>
        requires has_stream_traits<Subscription>
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_connect(const Subscription& sub)
    {
        return async_connect(stream_traits<Subscription>::target(cfg_, sub));
    }

    /// @brief Read and parse a single typed event.
    template<class Event>
    [[nodiscard]] boost::cobalt::task<result<Event>>
    async_read_event()
    {
        auto msg = co_await transport_.async_read_text();
        if (!msg) co_return result<Event>::failure(msg.err);
        Event event{};
        glz::context ctx{};
        if (auto ec = glz::read<fapi::detail::json_read_opts>(event, *msg, ctx))
            co_return result<Event>::failure({error_code::json, 0, 0, glz::format_error(ec, *msg), *msg});
        co_return result<Event>::success(std::move(event));
    }

    // -- Low-level --

    /// @brief Connect to a stream endpoint by raw target path.
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string target)
    {
        co_return co_await transport_.async_connect(cfg_.stream_host, cfg_.stream_port, std::move(target));
    }

    /// @brief Read a single raw text frame.
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text()
    {
        co_return co_await transport_.async_read_text();
    }

    /// @brief Close the stream connection.
    [[nodiscard]] boost::cobalt::task<result<void>> async_close()
    {
        co_return co_await transport_.async_close();
    }

    // -- Combined stream management --

    /// @brief Subscribe to stream topics on an existing combined connection.
    [[nodiscard]] boost::cobalt::task<result<void>> async_subscribe(const std::vector<std::string>& streams)
    {
        detail::stream_control_request req{ .method = "SUBSCRIBE", .params = streams, .id = 1 };
        auto json = glz::write_json(req);
        if (!json) co_return result<void>::failure({ error_code::json, 0, 0, "failed to serialize subscribe request", {} });

        auto wr = co_await transport_.async_write_text(*json);
        if (!wr) co_return result<void>::failure(wr.err);

        auto rd = co_await transport_.async_read_text();
        if (!rd) co_return result<void>::failure(rd.err);

        co_return result<void>::success();
    }

    /// @brief Unsubscribe from stream topics.
    [[nodiscard]] boost::cobalt::task<result<void>> async_unsubscribe(const std::vector<std::string>& streams)
    {
        detail::stream_control_request req{ .method = "UNSUBSCRIBE", .params = streams, .id = 2 };
        auto json = glz::write_json(req);
        if (!json) co_return result<void>::failure({ error_code::json, 0, 0, "failed to serialize unsubscribe request", {} });

        auto wr = co_await transport_.async_write_text(*json);
        if (!wr) co_return result<void>::failure(wr.err);

        auto rd = co_await transport_.async_read_text();
        if (!rd) co_return result<void>::failure(rd.err);

        co_return result<void>::success();
    }

    /// @brief List active subscriptions.
    [[nodiscard]] boost::cobalt::task<result<std::vector<std::string>>> async_list_subscriptions()
    {
        detail::stream_control_request req{ .method = "LIST_SUBSCRIPTIONS", .params = {}, .id = 3 };
        auto json = glz::write_json(req);
        if (!json) co_return result<std::vector<std::string>>::failure({ error_code::json, 0, 0, "failed to serialize list request", {} });

        auto wr = co_await transport_.async_write_text(*json);
        if (!wr) co_return result<std::vector<std::string>>::failure(wr.err);

        auto rd = co_await transport_.async_read_text();
        if (!rd) co_return result<std::vector<std::string>>::failure(rd.err);

        detail::stream_list_response response{};
        glz::context ctx{};
        if (glz::read<fapi::detail::json_read_opts>(response, *rd, ctx)) {
            co_return result<std::vector<std::string>>::failure({ error_code::json, 0, 0, "failed to parse list response", *rd });
        }

        co_return result<std::vector<std::string>>::success(std::move(response.result));
    }

    /// @brief Access the stream config.
    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }

    /// @brief Access the underlying transport.
    [[nodiscard]] Transport& transport() noexcept { return transport_; }

private:
    Transport transport_;
    config cfg_;
};

/// @brief Default market streams using the WebSocket transport.
using market_streams = basic_market_streams<>;

} // namespace binapi2::fapi::streams
