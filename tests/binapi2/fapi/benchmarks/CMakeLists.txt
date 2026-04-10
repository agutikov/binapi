# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures stream parsing benchmarks.

add_executable(stream_parse_benchmark stream_parse_benchmark.cpp)
target_link_libraries(stream_parse_benchmark PRIVATE binapi2_fapi)
target_compile_definitions(stream_parse_benchmark PRIVATE
    FIXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    TESTNET_STREAMS_PATH="${CMAKE_SOURCE_DIR}/testnet_output/streams"
)
