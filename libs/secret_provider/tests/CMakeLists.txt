# SPDX-License-Identifier: Apache-2.0

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(secret_provider_test secret_provider_test.cpp)
target_link_libraries(secret_provider_test PRIVATE secret_provider GTest::gtest_main)
gtest_discover_tests(secret_provider_test DISCOVERY_MODE PRE_TEST)
