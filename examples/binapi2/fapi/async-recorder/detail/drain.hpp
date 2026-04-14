// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor drain-and-route loop.

/// @file detail/drain.hpp

#pragma once

#include "types.hpp"

#include <boost/cobalt/task.hpp>

namespace binapi2::examples::async_recorder::detail_impl {

/// @brief Consume raw frames from `st.record_buf`, extract the
///        combined-endpoint `"stream"` header, and route each frame to
///        the right per-symbol rotating sink. Returns when the buffer
///        is closed.
boost::cobalt::task<void> drain_loop(detail_state& st);

} // namespace binapi2::examples::async_recorder::detail_impl
