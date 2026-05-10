// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file combined_market_stream.hpp
/// @brief Multiple market data subscriptions on a single WebSocket connection.
///
/// Connects to the /stream combined endpoint, subscribes to multiple topics,
/// and yields typed events as a caller-specified std::variant.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/detail/variant_parse.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/detail/stream_connection.hpp>
#include <binapi2/fapi/streams/detail/stream_consumer.hpp>
#include <binapi2/fapi/streams/stream_traits.hpp>
#include <binapi2/fapi/types/event_traits.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>
#include <glaze/glaze.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace binapi2::fapi::streams {

namespace detail {

/// @brief Build enum-keyed dispatch entry for a subscription's event type.
template<class Subscription, typename Variant>
    requires has_stream_traits<Subscription>
constexpr fapi::detail::variant_entry<types::market_event_type_t, Variant>
make_combined_entry()
{
    using Event = typename stream_traits<Subscription>::event_type;
    return fapi::detail::make_entry<Event, types::market_event_type_t, Variant>(
        types::event_traits<Event>::enum_value);
}

} // namespace detail

/// @brief Combined market data stream — multiple subscriptions on one connection.
///
/// Connects to the Binance /stream combined endpoint. Subscribes to multiple
/// topics and yields typed events as a caller-specified std::variant.
///
/// Usage:
///   using events_t = std::variant<book_ticker_stream_event_t, depth_stream_event_t>;
///   auto gen = combined.subscribe<events_t>(
///       book_ticker_subscription{.symbol = "BTCUSDT"},
///       diff_book_depth_subscription{.symbol = "BTCUSDT", .speed = "100ms"});
///   while (gen) {
///       auto ev = co_await gen;
///       if (!ev) break;
///       std::visit(overloaded{...}, *ev);
///   }
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client,
         stream_consumer Consumer = inline_consumer>
class basic_combined_market_stream
{
public:
    /// Construct a stream pinned to a Binance URL category. The category
    /// determines the combined-stream URL (`/public/stream` or
    /// `/market/stream`) and the SUBSCRIBE compile-time check that
    /// rejects mixing categories.
    explicit basic_combined_market_stream(config cfg, stream_category_e category) :
        conn_(std::move(cfg)), category_(category)
    {
    }

    basic_combined_market_stream(config cfg, stream_category_e category, Consumer consumer) :
        conn_(std::move(cfg), std::move(consumer)), category_(category)
    {
    }

    /// Legacy single-arg constructor — defaults to `/public` for
    /// source-compat with pre-2026 callers. New code should pick a
    /// category explicitly.
    [[deprecated("pass a stream_category_e explicitly; see stream_category.hpp")]]
    explicit basic_combined_market_stream(config cfg) :
        basic_combined_market_stream(std::move(cfg), stream_category_e::public_)
    {
    }

    [[deprecated("pass a stream_category_e explicitly; see stream_category.hpp")]]
    basic_combined_market_stream(config cfg, Consumer consumer) :
        basic_combined_market_stream(std::move(cfg), stream_category_e::public_, std::move(consumer))
    {
    }

    basic_combined_market_stream(const basic_combined_market_stream&) = delete;
    basic_combined_market_stream& operator=(const basic_combined_market_stream&) = delete;

    /// Category this stream connects under.
    [[nodiscard]] stream_category_e category() const noexcept { return category_; }

    // -- Subscribe and consume --

    /// @brief Connect, subscribe, and yield typed variant events with auto-reconnect.
    /// `same_category` rejects mixing categories at compile time.
    template<typename Variant, class First, class... Rest>
        requires same_category<First, Rest...>
    boost::cobalt::generator<result<Variant>>
    subscribe(const First& first, const Rest&... rest)
    {
        // Build combined target: /<category>/stream?streams=topic1/topic2/...
        std::string topics_path = stream_traits<First>::topic(conn_.configuration(), first);
        ((topics_path += "/" + stream_traits<Rest>::topic(conn_.configuration(), rest)), ...);
        const auto host = conn_.configuration().stream_host;
        const auto port = conn_.configuration().stream_port;
        const ws_target_t target =
            std::string{ category_combined_target(category_) } + "?streams=" + topics_path;

        // Dispatch table: enum → parse function
        constexpr fapi::detail::variant_entry<types::market_event_type_t, Variant> dispatch[] = {
            detail::make_combined_entry<First, Variant>(),
            detail::make_combined_entry<Rest, Variant>()...
        };

        error last_err;
        int failures = 0;

        while (failures <= max_reconnects_) {
            auto conn = co_await conn_.async_connect(host, port, target);
            if (!conn) {
                last_err = conn.err;
                ++failures;
                continue;
            }

            failures = 0;

            while (true) {
                auto msg = co_await conn_.async_read_text();
                if (!msg) {
                    last_err = msg.err;
                    co_await conn_.async_close();
                    ++failures;
                    break;
                }

                types::combined_stream_frame_t frame{};
                glz::context ctx{};
                if (glz::read<fapi::detail::json_read_opts>(frame, *msg, ctx))
                    continue;

                if (frame.stream.empty()) continue;

                auto parsed = fapi::detail::parse_variant<types::market_event_type_t, Variant>(
                    "e", frame.data.str, dispatch);
                if (parsed) {
                    co_yield result<Variant>::success(std::move(*parsed));
                } else {
                    co_yield result<Variant>::failure(
                        {error_code::json, 0, 0,
                         "parse failed for topic: " + frame.stream, std::string(frame.data.str)});
                    failures = max_reconnects_ + 1;
                    break;
                }
            }
        }

        co_yield result<Variant>::failure(last_err);
    }

    /// @brief Set maximum consecutive reconnect attempts (default: 3).
    void set_max_reconnects(int n) { max_reconnects_ = n; }

    // -- Accessors --

    [[nodiscard]] Transport& transport() noexcept { return conn_.transport(); }
    [[nodiscard]] basic_stream_connection<Transport, Consumer>& connection() noexcept { return conn_; }

private:
    basic_stream_connection<Transport, Consumer> conn_;
    stream_category_e category_{ stream_category_e::public_ };
    int max_reconnects_{3};
};

using combined_market_stream = basic_combined_market_stream<>;

} // namespace binapi2::fapi::streams
