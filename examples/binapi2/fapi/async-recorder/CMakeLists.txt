# SPDX-License-Identifier: Apache-2.0
#
# binapi2-fapi-async-recorder: institutional-grade recorder example.
# See docs/binapi2/plans/async_recorder.md for the design.

find_package(spdlog REQUIRED)

add_executable(binapi2-fapi-async-recorder
    main.cpp
    config.cpp
)
target_link_libraries(binapi2-fapi-async-recorder
    PRIVATE
        binapi2_fapi
        spdlog::spdlog
)
