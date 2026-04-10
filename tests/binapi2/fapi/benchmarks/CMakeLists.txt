# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures benchmarks.

add_executable(stream_parse_benchmark stream_parse_benchmark.cpp)
target_link_libraries(stream_parse_benchmark PRIVATE binapi2_fapi)
target_compile_definitions(stream_parse_benchmark PRIVATE
    FIXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

add_executable(rest_benchmark rest_benchmark.cpp)
target_link_libraries(rest_benchmark PRIVATE binapi2_fapi)
