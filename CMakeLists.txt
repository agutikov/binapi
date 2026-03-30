cmake_minimum_required(VERSION 3.5)
project(binapi)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_definitions(
    -UNDEBUG
    -DDTF_HEADER_ONLY
)

include_directories(
    ./include
)

if (DEFINED ${BOOST_INCLUDE_DIR})
    include_directories(
        ${BOOST_INCLUDE_DIR}
    )
endif()

set(BINAPI_HEADERS
    include/binapi/api.hpp
    include/binapi/flatjson.hpp
    include/binapi/dtf.hpp
    include/binapi/double_type.hpp
    include/binapi/enums.hpp
    include/binapi/errors.hpp
    include/binapi/invoker.hpp
    include/binapi/message.hpp
    include/binapi/pairslist.hpp
    include/binapi/reports.hpp
    include/binapi/tools.hpp
    include/binapi/types.hpp
    include/binapi/websocket.hpp
)

set(BINAPI_SOURCES
    src/api.cpp
    src/enums.cpp
    src/errors.cpp
    src/pairslist.cpp
    src/reports.cpp
    src/tools.cpp
    src/types.cpp
    src/websocket.cpp
)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(deps)

set(BINAPI2_UMF_HEADERS
    include/binapi2/umf/client.hpp
    include/binapi2/umf/config.hpp
    include/binapi2/umf/error.hpp
    include/binapi2/umf/result.hpp
    include/binapi2/umf/signing.hpp
    include/binapi2/umf/time.hpp
    include/binapi2/umf/rest/account.hpp
    include/binapi2/umf/rest/generated_endpoints.hpp
    include/binapi2/umf/rest/market_data.hpp
    include/binapi2/umf/rest/trade.hpp
    include/binapi2/umf/rest/user_data_streams.hpp
    include/binapi2/umf/streams/market_streams.hpp
    include/binapi2/umf/streams/subscriptions.hpp
    include/binapi2/umf/streams/user_streams.hpp
    include/binapi2/umf/transport/http_client.hpp
    include/binapi2/umf/transport/session_base.hpp
    include/binapi2/umf/transport/ssl_context.hpp
    include/binapi2/umf/transport/websocket_client.hpp
    include/binapi2/umf/types/common.hpp
    include/binapi2/umf/types/account.hpp
    include/binapi2/umf/types/enums.hpp
    include/binapi2/umf/types/market_data.hpp
    include/binapi2/umf/types/streams.hpp
    include/binapi2/umf/types/trade.hpp
    include/binapi2/umf/types/websocket_api.hpp
    include/binapi2/umf/websocket_api/client.hpp
    include/binapi2/umf/websocket_api/generated_methods.hpp
    include/binapi2/umf/websocket_api/methods.hpp
)

set(BINAPI2_UMF_SOURCES
    src/binapi2/umf/client.cpp
    src/binapi2/umf/rest.cpp
    src/binapi2/umf/signing.cpp
    src/binapi2/umf/transport/http_client.cpp
    src/binapi2/umf/transport/ssl_context.cpp
    src/binapi2/umf/transport/websocket_client.cpp
    src/binapi2/umf/websocket_api/client.cpp
    src/binapi2/umf/streams/market_streams.cpp
    src/binapi2/umf/streams/user_streams.cpp
)

add_library(
    ${PROJECT_NAME} STATIC
    ${BINAPI_SOURCES}
    ${BINAPI_HEADERS}
)

add_library(
    binapi2_umf STATIC
    ${BINAPI2_UMF_SOURCES}
    ${BINAPI2_UMF_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(binapi2_umf
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (DEFINED ${BOOST_LIB_DIR})
    target_link_libraries(
        ${PROJECT_NAME}
        -L${BOOST_LIB_DIR}
    )
endif()

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    ZLIB::ZLIB
    OpenSSL::Crypto
    OpenSSL::SSL
    Threads::Threads
)

target_link_libraries(
    binapi2_umf
    PUBLIC
        glaze::glaze
        ZLIB::ZLIB
        OpenSSL::Crypto
        OpenSSL::SSL
        Threads::Threads
)

add_executable(
    ${PROJECT_NAME}_app
    main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_app
    PRIVATE
    ${PROJECT_NAME}
)

add_executable(binapi2_umf_rest_ping examples/binapi2/umf/rest_ping.cpp)
target_link_libraries(binapi2_umf_rest_ping PRIVATE binapi2_umf)

add_executable(binapi2_umf_rest_exchange_info examples/binapi2/umf/rest_exchange_info.cpp)
target_link_libraries(binapi2_umf_rest_exchange_info PRIVATE binapi2_umf)

add_executable(binapi2_umf_rest_new_order examples/binapi2/umf/rest_new_order.cpp)
target_link_libraries(binapi2_umf_rest_new_order PRIVATE binapi2_umf)

add_executable(binapi2_umf_websocket_api_session_logon examples/binapi2/umf/websocket_api_session_logon.cpp)
target_link_libraries(binapi2_umf_websocket_api_session_logon PRIVATE binapi2_umf)

add_executable(binapi2_umf_stream_book_ticker examples/binapi2/umf/stream_book_ticker.cpp)
target_link_libraries(binapi2_umf_stream_book_ticker PRIVATE binapi2_umf)

add_executable(binapi2_umf_user_stream_listen_key examples/binapi2/umf/user_stream_listen_key.cpp)
target_link_libraries(binapi2_umf_user_stream_listen_key PRIVATE binapi2_umf)

add_executable(binapi2_umf_rest_account_information examples/binapi2/umf/rest_account_information.cpp)
target_link_libraries(binapi2_umf_rest_account_information PRIVATE binapi2_umf)

add_executable(binapi2_umf_websocket_api_account_status examples/binapi2/umf/websocket_api_account_status.cpp)
target_link_libraries(binapi2_umf_websocket_api_account_status PRIVATE binapi2_umf)

add_executable(binapi2_umf_stream_mark_price examples/binapi2/umf/stream_mark_price.cpp)
target_link_libraries(binapi2_umf_stream_mark_price PRIVATE binapi2_umf)
