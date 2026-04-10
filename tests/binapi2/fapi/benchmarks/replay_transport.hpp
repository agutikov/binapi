// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures stream parsing benchmarks.

/// @file replay_transport.hpp
/// @brief Fake WebSocket transport that replays pre-loaded JSON strings.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>

#include <boost/asio/post.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <string>
#include <vector>

namespace binapi2::fapi::benchmarks {

/// @brief Transport that replays a vector of JSON messages without I/O.
///
/// Satisfies the same interface as transport::websocket_client so it can
/// be used with basic_market_stream<replay_transport> and
/// basic_user_stream<replay_transport>.
class replay_transport
{
public:
    explicit replay_transport(config) {}

    std::vector<std::string> messages;
    std::size_t index = 0;

    void reset() { index = 0; }

    boost::cobalt::task<result<void>> async_connect(std::string, std::string, ws_target_t)
    {
        co_return result<void>::success();
    }

    boost::cobalt::task<result<void>> async_write_text(std::string)
    {
        co_return result<void>::success();
    }

    boost::cobalt::task<result<std::string>> async_read_text()
    {
        // Yield to the event loop to prevent stack overflow from synchronous
        // inline completion of 1000+ consecutive co_awaits.
        co_await boost::asio::post(co_await boost::cobalt::this_coro::executor, boost::cobalt::use_op);

        if (index < messages.size())
            co_return result<std::string>::success(std::string(messages[index++]));
        co_return result<std::string>::failure({error_code::websocket, 0, 0, "end of replay", {}});
    }

    boost::cobalt::task<result<void>> async_close()
    {
        co_return result<void>::success();
    }
};

static_assert(transport::websocket_transport<replay_transport>);

} // namespace binapi2::fapi::benchmarks
