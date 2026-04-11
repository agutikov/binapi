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

add_executable(stream_buffer_benchmark stream_buffer_benchmark.cpp)
target_link_libraries(stream_buffer_benchmark PRIVATE binapi2_fapi)

find_package(spdlog REQUIRED)

add_executable(stream_recorder_benchmark stream_recorder_benchmark.cpp)
target_link_libraries(stream_recorder_benchmark PRIVATE binapi2_fapi spdlog::spdlog uring)
target_compile_definitions(stream_recorder_benchmark PRIVATE BOOST_ASIO_HAS_IO_URING)
