# SPDX-License-Identifier: Apache-2.0
#
# binapi2_demo_commands — header-only library shared between the
# `async-demo-cli` and `async-demo-ui` example binaries.
#
# Contains:
#   * `result_sink`        — abstract sink interface for command output
#   * `spdlog_sink`        — header-only spdlog-based implementation (CLI)
#   * `enum_parse`         — glaze-backed enum string parser
#   * `opts`               — POD option structs for shared parameter shapes
#   * `builders`           — make_*_request helpers turning opts into Requests
#   * `exec`               — sink-parameterised REST / WS API / stream executors
#
# The library does not link spdlog itself — spdlog_sink.hpp #includes spdlog,
# so consumers that use it must link spdlog themselves.

add_library(binapi2_demo_commands INTERFACE)
add_library(binapi2::demo_commands ALIAS binapi2_demo_commands)

target_include_directories(binapi2_demo_commands
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(binapi2_demo_commands
    INTERFACE
        binapi2_fapi
        glaze::glaze
)

target_compile_features(binapi2_demo_commands INTERFACE cxx_std_23)
