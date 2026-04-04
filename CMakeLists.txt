cmake_minimum_required(VERSION 3.10)
project(binapi)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS ON) # Enables gnu++23 instead of c++23

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

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

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(deps)

if (DEFINED ${BOOST_LIB_DIR})
    target_link_libraries(
        ${PROJECT_NAME}
        -L${BOOST_LIB_DIR}
    )
endif()

add_subdirectory(src/binapi)
add_subdirectory(src/binapi2/fapi)
add_subdirectory(examples/binapi)
add_subdirectory(examples/binapi2/fapi)

option(BINAPI2_WITH_TESTS "Build binapi2 tests" ON)
if(BINAPI2_WITH_TESTS)
    enable_testing()
    add_subdirectory(tests/binapi2/fapi)
endif()

if(PROJECT_IS_TOP_LEVEL AND UNIX)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()
