// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor string / config / time helpers.

/// @file detail/helpers.hpp

#pragma once

#include "../config.hpp"

#include <binapi2/fapi/streams/detail/sinks/rotating_file_sink.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace binapi2::examples::async_recorder::detail_impl {

/// @brief Build a rotating-file-sink config for one detail sink, under
///        `<root>/detail/<SYM>/<stream_type>/`. Creates the directory.
::binapi2::fapi::streams::sinks::rotating_file_sink_config
make_detail_rfs_cfg(const recorder_config& cfg,
                    const std::string& symbol_upper,
                    const std::string& stream_type);

/// @brief Extract the combined-endpoint stream header from a raw frame,
///        returning `{symbol_lower, stream_type}`. Cheap substring scan,
///        no full JSON parse. Returns nullopt if the frame isn't in
///        `{"stream":"X@Y",...}` shape.
std::optional<std::pair<std::string, std::string>>
parse_stream_header(std::string_view frame);

/// @brief Upper-case copy of an ASCII string.
std::string to_upper_copy(std::string_view s);

/// @brief UTC ISO-8601 with milliseconds: `YYYY-MM-DDTHH:MM:SS.mmmZ`.
std::string now_iso_ms();

/// @brief UTC filename timestamp: `YYYYMMDDTHHMMSSZ`.
std::string now_filename_ts();

} // namespace binapi2::examples::async_recorder::detail_impl
