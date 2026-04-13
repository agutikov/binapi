// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_parser.hpp
/// @brief Generic async parse stage for stream pipelines.
///
/// Reads raw JSON strings from an input buffer, parses each into a typed
/// event using a user-provided parse function, and pushes the result into
/// an output buffer. Runs as a coroutine on any executor.

#pragma once

#include <binapi2/fapi/detail/stream_buffer_consumer.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>
#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/task.hpp>

#include <functional>
#include <string>

namespace binapi2::fapi::streams {

/// @brief Generic parse stage: reads strings, parses, pushes typed events.
///
/// Usage:
///   threadsafe_stream_buffer<string> raw_buf(4096);
///   threadsafe_stream_buffer<depth_stream_event_t> event_buf(4096);
///
///   stream_parser<depth_stream_event_t> parser(raw_buf, event_buf,
///       [](const string& s) { return parse_depth(s); });
///
///   // Run on a dedicated io_thread:
///   cobalt::spawn(parser_io.context(), parser.async_run(), use_future);
///
/// @tparam Event  Typed event produced by the parse function.
template<typename Event>
class stream_parser
{
public:
    using parse_fn = std::function<result<Event>(const std::string&)>;

    stream_parser(fapi::detail::threadsafe_stream_buffer<std::string>& input,
                  fapi::detail::threadsafe_stream_buffer<Event>& output,
                  parse_fn parse) :
        input_(input), output_(output), parse_(std::move(parse))
    {
    }

    stream_parser(const stream_parser&) = delete;
    stream_parser& operator=(const stream_parser&) = delete;

    /// @brief Run the parse loop.
    ///
    /// Reads strings from input, parses each, pushes successful results
    /// to output. Runs the input forwarder concurrently. Exits when
    /// the input buffer is closed and drained.
    [[nodiscard]] boost::cobalt::task<void> async_run()
    {
        auto parse_loop = [this]() -> boost::cobalt::task<void> {
            while (true) {
                auto raw = co_await input_.async_read();
                if (!raw) break;

                auto event = parse_(*raw);
                if (event) {
                    while (!output_.push(std::move(*event))) {}
                }
            }
            output_.close();
        };

        co_await boost::cobalt::join(input_.async_forward(), parse_loop());
    }

private:
    fapi::detail::threadsafe_stream_buffer<std::string>& input_;
    fapi::detail::threadsafe_stream_buffer<Event>& output_;
    parse_fn parse_;
};

} // namespace binapi2::fapi::streams
