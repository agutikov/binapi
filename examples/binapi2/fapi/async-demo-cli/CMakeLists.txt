# SPDX-License-Identifier: Apache-2.0
#
# binapi2-fapi-async-demo-cli: async demonstration client and library usage example.

find_package(spdlog REQUIRED)

add_executable(binapi2-fapi-async-demo-cli
    main.cpp
    common.cpp
    cmd_market_data.cpp
    cmd_account.cpp
    cmd_trade.cpp
    cmd_ws_api.cpp
    cmd_stream.cpp
    cmd_user_stream.cpp
    cmd_order_book.cpp
    cmd_pipeline_order_book.cpp
)
target_link_libraries(binapi2-fapi-async-demo-cli PRIVATE binapi2_fapi spdlog::spdlog)
