cmake_minimum_required(VERSION 3.5)
project(binapi)

set(CMAKE_CXX_STANDARD 23)

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

add_library(
    ${PROJECT_NAME} STATIC
    ${BINAPI_SOURCES}
    ${BINAPI_HEADERS}
)

target_include_directories(${PROJECT_NAME}
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

add_executable(
    ${PROJECT_NAME}_app
    main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_app
    PRIVATE
    ${PROJECT_NAME}
)
