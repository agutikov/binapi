# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures client library.

set(BINAPI2_FAPI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/config.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/error.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/result.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/signing.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/time.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/account.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/generated_endpoints.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/market_data.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/trade.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/user_data_streams.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/market_streams.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/subscriptions.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/user_streams.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/http_client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/session_base.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/ssl_context.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/websocket_client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/common.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/account.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/enums.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/market_data.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/streams.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/trade.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/websocket_api.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/websocket_api/client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/websocket_api/generated_methods.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/websocket_api/methods.hpp
)

set(BINAPI2_FAPI_SOURCES
    client.cpp
    rest/account.cpp
    rest/common.cpp
    rest/market_data.cpp
    rest/trade.cpp
    rest/user_data_streams.cpp
    signing.cpp
    transport/http_client.cpp
    transport/ssl_context.cpp
    transport/websocket_client.cpp
    websocket_api/client.cpp
    streams/market_streams.cpp
    streams/user_streams.cpp
)

add_library(
    binapi2_fapi STATIC
    ${BINAPI2_FAPI_SOURCES}
    ${BINAPI2_FAPI_HEADERS}
)

target_include_directories(binapi2_fapi
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    binapi2_fapi
    PUBLIC
        glaze::glaze
        ZLIB::ZLIB
        OpenSSL::Crypto
        OpenSSL::SSL
        Threads::Threads
)
