# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures tests.

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(decimal_test decimal_test.cpp)
target_link_libraries(decimal_test PRIVATE binapi2_fapi GTest::gtest_main)
# Relax -Werror for test code so gtest headers don't break the build.
target_compile_options(decimal_test PRIVATE)
gtest_discover_tests(decimal_test DISCOVERY_MODE PRE_TEST)

add_executable(result_test result_test.cpp)
target_link_libraries(result_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(result_test PRIVATE)
gtest_discover_tests(result_test DISCOVERY_MODE PRE_TEST)

add_executable(signing_test signing_test.cpp)
target_link_libraries(signing_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(signing_test PRIVATE)
gtest_discover_tests(signing_test DISCOVERY_MODE PRE_TEST)

add_executable(enums_test enums_test.cpp)
target_link_libraries(enums_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(enums_test PRIVATE)
gtest_discover_tests(enums_test DISCOVERY_MODE PRE_TEST)

add_executable(enum_set_test enum_set_test.cpp)
target_link_libraries(enum_set_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(enum_set_test PRIVATE)
gtest_discover_tests(enum_set_test DISCOVERY_MODE PRE_TEST)

add_executable(json_query_test json_query_test.cpp)
target_link_libraries(json_query_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(json_query_test PRIVATE)
gtest_discover_tests(json_query_test DISCOVERY_MODE PRE_TEST)

add_executable(response_parse_test response_parse_test.cpp)
target_link_libraries(response_parse_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(response_parse_test PRIVATE)
target_compile_definitions(response_parse_test PRIVATE
    RESPONSES_PATH="${PROJECT_SOURCE_DIR}/compose/postman-mock/responses"
)
gtest_discover_tests(response_parse_test DISCOVERY_MODE PRE_TEST)

add_executable(postman_validation_test postman_validation_test.cpp)
target_link_libraries(postman_validation_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(postman_validation_test PRIVATE)
target_compile_definitions(postman_validation_test PRIVATE
    POSTMAN_JSON_PATH="${PROJECT_SOURCE_DIR}/docs/api/binance-api-postman/collections/Binance Derivatives Trading USDS Futures API.json"
)
gtest_discover_tests(postman_validation_test DISCOVERY_MODE PRE_TEST)

add_executable(io_thread_test io_thread_test.cpp)
target_link_libraries(io_thread_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(io_thread_test PRIVATE)
gtest_discover_tests(io_thread_test DISCOVERY_MODE PRE_TEST)

add_executable(stream_test stream_test.cpp)
target_link_libraries(stream_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(stream_test PRIVATE)
gtest_discover_tests(stream_test DISCOVERY_MODE PRE_TEST)

add_executable(stream_buffer_test stream_buffer_test.cpp)
target_link_libraries(stream_buffer_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(stream_buffer_test PRIVATE)
gtest_discover_tests(stream_buffer_test DISCOVERY_MODE PRE_TEST)

find_package(spdlog REQUIRED)

add_executable(stream_consumer_test stream_consumer_test.cpp)
target_link_libraries(stream_consumer_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(stream_consumer_test PRIVATE)
gtest_discover_tests(stream_consumer_test DISCOVERY_MODE PRE_TEST)

add_executable(stream_recorder_test stream_recorder_test.cpp)
target_link_libraries(stream_recorder_test PRIVATE binapi2_fapi spdlog::spdlog GTest::gtest_main)
target_compile_options(stream_recorder_test PRIVATE)
gtest_discover_tests(stream_recorder_test DISCOVERY_MODE PRE_TEST)

add_executable(rotating_file_sink_test rotating_file_sink_test.cpp)
target_link_libraries(rotating_file_sink_test PRIVATE binapi2_fapi GTest::gtest_main)
gtest_discover_tests(rotating_file_sink_test DISCOVERY_MODE PRE_TEST)

# async-recorder example: selector unit tests. Registered here (not in
# the example's own CMakeLists) because the example subdir is added
# before enable_testing(), so gtest_discover_tests would not enrol them
# under ctest from there.
set(_ar_dir ${PROJECT_SOURCE_DIR}/examples/binapi2/fapi/async-recorder)
add_executable(async_recorder_selector_test
    ${_ar_dir}/selector.cpp
    ${_ar_dir}/selector_test.cpp
)
target_include_directories(async_recorder_selector_test PRIVATE ${_ar_dir})
target_link_libraries(async_recorder_selector_test
    PRIVATE binapi2_fapi spdlog::spdlog GTest::gtest_main)
gtest_discover_tests(async_recorder_selector_test DISCOVERY_MODE PRE_TEST)

add_subdirectory(integration/postman_mock)
add_subdirectory(integration/sync_bridging)
add_subdirectory(benchmarks)
