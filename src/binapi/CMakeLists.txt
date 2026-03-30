set(BINAPI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/binapi/api.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/flatjson.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/dtf.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/double_type.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/enums.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/errors.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/invoker.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/message.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/pairslist.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/reports.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/tools.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/types.hpp
    ${CMAKE_SOURCE_DIR}/include/binapi/websocket.hpp
)

set(BINAPI_SOURCES
    api.cpp
    enums.cpp
    errors.cpp
    pairslist.cpp
    reports.cpp
    tools.cpp
    types.cpp
    websocket.cpp
)

add_library(
    ${PROJECT_NAME} STATIC
    ${BINAPI_SOURCES}
    ${BINAPI_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        ZLIB::ZLIB
        OpenSSL::Crypto
        OpenSSL::SSL
        Threads::Threads
)
