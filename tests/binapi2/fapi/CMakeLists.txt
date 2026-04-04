# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures tests.

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(decimal_test decimal_test.cpp)
target_link_libraries(decimal_test PRIVATE binapi2_fapi GTest::gtest_main)
# Relax -Werror for test code so gtest headers don't break the build.
target_compile_options(decimal_test PRIVATE -Wno-error)
gtest_discover_tests(decimal_test)
