# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures client library.

set(BINAPI2_FAPI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/binapi2/futures_usdm_api.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/config.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/error.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/result.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/signing.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/time.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/services/account.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/generated_endpoints.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/services/convert.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/services/market_data.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/services/trade.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/rest/services/user_data_streams.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/stream_connection.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/combined_market_stream.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/dynamic_market_stream.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/market_stream.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/order_book/local_order_book.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/streams/user_stream.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/http_client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/session_base.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/ssl_context.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/transport/websocket_client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/detail/decimal.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/detail/enum_set.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/detail/pair.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/detail/symbol.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/subscriptions.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/common.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/account.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/convert.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/enums.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/market_data.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/market_stream_events.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/user_stream_events.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/trade.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/types/websocket_api.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/websocket_api/client.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi2/fapi/websocket_api/generated_methods.hpp
)

set(BINAPI2_FAPI_SOURCES
    decimal.cpp
    client.cpp
    rest/client.cpp
    rest/pipeline.cpp
    rest/trade.cpp
    signing.cpp
    transport/http_client.cpp
    transport/ssl_context.cpp
    transport/websocket_client.cpp
    websocket_api/client.cpp
    streams/stream_connection.cpp
    order_book/local_order_book.cpp
    streams/market_stream.cpp
    streams/user_stream.cpp
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

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED COMPONENTS cobalt)

target_link_libraries(
    binapi2_fapi
    PUBLIC
        glaze::glaze
        Boost::cobalt
        ZLIB::ZLIB
        OpenSSL::Crypto
        OpenSSL::SSL
        Threads::Threads
)
