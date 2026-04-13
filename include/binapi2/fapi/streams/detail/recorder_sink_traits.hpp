// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file recorder_sink_traits.hpp
/// @brief Shared sink-kind detection for stream recorder templates.

#pragma once

#include <boost/cobalt/task.hpp>

#include <string>
#include <type_traits>

namespace binapi2::fapi::streams::detail {

/// @brief True when Sink::operator()(const string&) returns cobalt::task<void>.
template<typename Sink, typename = void>
struct is_async_sink : std::false_type
{
};

template<typename Sink>
struct is_async_sink<
    Sink,
    std::void_t<decltype(std::declval<Sink&>()(std::declval<const std::string&>()))>>
    : std::is_same<
          boost::cobalt::task<void>,
          decltype(std::declval<Sink&>()(std::declval<const std::string&>()))>
{
};

} // namespace binapi2::fapi::streams::detail
