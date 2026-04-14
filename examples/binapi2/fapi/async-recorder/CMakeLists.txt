# SPDX-License-Identifier: Apache-2.0
#
# binapi2-fapi-async-recorder: institutional-grade recorder example.
# See docs/binapi2/plans/async_recorder.md for the design.

find_package(spdlog REQUIRED)

add_executable(binapi2-fapi-async-recorder
    main.cpp
    config.cpp
    screener.cpp
    status_reporter.cpp
)
target_link_libraries(binapi2-fapi-async-recorder
    PRIVATE
        binapi2_fapi
        spdlog::spdlog
)
# BOOST_ASIO_HAS_IO_URING / BOOST_ASIO_DISABLE_EPOLL and -luring propagate
# publicly from binapi2_fapi (see src/binapi2/fapi/CMakeLists.txt).
