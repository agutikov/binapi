# SPDX-License-Identifier: Apache-2.0
#
# binapi2-fapi-async-recorder: institutional-grade recorder example.
# See docs/binapi2/plans/async_recorder.md for the design.

find_package(spdlog REQUIRED)

add_executable(binapi2-fapi-async-recorder
    main.cpp
    config.cpp
    screener.cpp
    selector.cpp
    selector_run.cpp
    status_reporter.cpp
)
target_link_libraries(binapi2-fapi-async-recorder
    PRIVATE
        binapi2_fapi
        spdlog::spdlog
)
# BOOST_ASIO_HAS_IO_URING propagates publicly from binapi2_fapi
# (see src/binapi2/fapi/CMakeLists.txt).

# The selector unit test executable is built and registered from
# tests/binapi2/fapi/CMakeLists.txt (which runs after enable_testing())
# so `gtest_discover_tests` picks it up under ctest.
