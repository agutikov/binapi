// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor REST depth snapshot.

/// @file detail/snapshot.hpp
/// @brief REST depth-snapshot pull, used on symbol admission and
///        (later, F4) as a periodic re-anchor for the diff stream.

#pragma once

#include <binapi2/fapi/rest/client.hpp>

#include <boost/cobalt/task.hpp>

#include <filesystem>
#include <string>
#include <string_view>

namespace binapi2::examples::async_recorder::detail_impl {

/// @brief Fetch a 1000-level depth snapshot via REST and write it as a
///        single `{"t":"<iso>","d":<response>}` line to
///        `<root>/detail/<SYM>/depth_snapshot/<tag>.<UTCts>.json`.
///        Returns true on success so callers can tally `ok/err`.
///
/// @param tag  Filename prefix. `"startup"` on admission; `"resnap"`
///             (F4) for the periodic re-anchor.
boost::cobalt::task<bool>
fetch_depth_snapshot(::binapi2::fapi::rest::client& client,
                     const std::filesystem::path& root,
                     const std::string& symbol_upper,
                     std::string_view tag);

} // namespace binapi2::examples::async_recorder::detail_impl
