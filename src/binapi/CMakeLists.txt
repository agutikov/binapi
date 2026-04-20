set(BINAPI_HEADERS
    ${PROJECT_SOURCE_DIR}/include/binapi/api.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/flatjson.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/dtf.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/double_type.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/enums.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/errors.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/invoker.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/message.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/pairslist.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/reports.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/tools.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/types.hpp
    ${PROJECT_SOURCE_DIR}/include/binapi/websocket.hpp
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
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        ZLIB::ZLIB
        OpenSSL::Crypto
        OpenSSL::SSL
        Threads::Threads
)
