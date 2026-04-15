# SPDX-License-Identifier: Apache-2.0
#
# binapi2-fapi-async-demo-ui — FTXUI-based interactive demonstration client.

find_package(spdlog REQUIRED)

add_executable(binapi2-fapi-async-demo-ui
    main.cpp
    worker.cpp
    views/status_bar.cpp
    views/rest_view.cpp
    views/ws_view.cpp
    views/streams_view.cpp
    views/orderbook_view.cpp
)

target_link_libraries(binapi2-fapi-async-demo-ui
    PRIVATE
        binapi2_fapi
        binapi2_demo_commands
        secret_provider
        spdlog::spdlog
        CLI11::CLI11
        ftxui::component
        ftxui::dom
        ftxui::screen
)
