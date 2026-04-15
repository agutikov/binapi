add_executable(binapi2-fapi-sync-demo
    main.cpp
    rest_blocking.cpp
    rest_future.cpp
    rest_callback.cpp
    ws_api_blocking.cpp
    ws_api_future.cpp
    ws_api_callback.cpp
    stream_blocking.cpp
    stream_callback.cpp
)
target_link_libraries(binapi2-fapi-sync-demo PRIVATE binapi2_fapi CLI11::CLI11)
