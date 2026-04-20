# SPDX-License-Identifier: Apache-2.0
#
# binapi2 USD-M Futures client library.

set(BINAPI2_FAPI_HEADERS
    ${PROJECT_SOURCE_DIR}/include/binapi2/futures_usdm_api.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/client.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/config.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/error.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/result.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/signing.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/time.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/services/account.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/generated_endpoints.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/services/convert.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/services/market_data.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/services/trade.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/rest/services/user_data_streams.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/stream_connection.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/combined_market_stream.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/dynamic_market_stream.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/threaded_stream_recorder.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/async_stream_recorder.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/recorder_sink_traits.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/stream_consumer.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/stream_parser.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/sinks/callback_sink.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/sinks/spdlog_sink.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/sinks/file_sink.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/detail/sinks/rotating_file_sink.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/market_stream.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/order_book/local_order_book.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/streams/user_stream.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/transport/http_client.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/transport/session_base.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/transport/ssl_context.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/transport/websocket_client.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/detail/decimal.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/detail/enum_set.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/detail/pair.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/detail/symbol.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/subscriptions.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/common.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/account.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/convert.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/enums.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/market_data.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/market_stream_events.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/user_stream_events.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/trade.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/types/websocket_api.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/websocket_api/client.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi2/fapi/websocket_api/generated_methods.hpp
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
    order_book/pipeline_order_book.cpp
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
        ${PROJECT_SOURCE_DIR}/include
)

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED COMPONENTS cobalt)

# Boost.Asio reactor defines must be propagated to every translation unit
# that includes <boost/asio/*.hpp> — otherwise different TUs pick
# different reactor impls and mixing them in one io_context is UB.
# We enable io_uring (needed for asio::stream_file used by file_sink) and
# leave epoll for sockets/timers; flipping BOOST_ASIO_DISABLE_EPOLL on top
# breaks websocket reads on this host.
target_compile_definitions(binapi2_fapi
    PUBLIC
        BOOST_ASIO_HAS_IO_URING
)

target_link_libraries(
    binapi2_fapi
    PUBLIC
        glaze::glaze
        Boost::cobalt
        ZLIB::ZLIB
        OpenSSL::Crypto
        OpenSSL::SSL
        Threads::Threads
        uring
)
