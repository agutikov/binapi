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
    RESPONSES_PATH="${CMAKE_SOURCE_DIR}/compose/postman-mock/responses"
)
gtest_discover_tests(response_parse_test DISCOVERY_MODE PRE_TEST)

add_executable(postman_validation_test postman_validation_test.cpp)
target_link_libraries(postman_validation_test PRIVATE binapi2_fapi GTest::gtest_main)
target_compile_options(postman_validation_test PRIVATE)
target_compile_definitions(postman_validation_test PRIVATE
    POSTMAN_JSON_PATH="${CMAKE_SOURCE_DIR}/docs/api/binance-api-postman/collections/Binance Derivatives Trading USDS Futures API.json"
)
gtest_discover_tests(postman_validation_test DISCOVERY_MODE PRE_TEST)

add_subdirectory(integration/postman_mock)
add_subdirectory(integration/sync_bridging)
