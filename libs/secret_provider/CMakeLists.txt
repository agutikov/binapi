# SPDX-License-Identifier: Apache-2.0
#
# secret_provider — generic credential provider library.

find_package(Boost REQUIRED COMPONENTS cobalt)
find_package(PkgConfig)
pkg_check_modules(LIBSECRET libsecret-1)

set(SECRET_PROVIDER_SOURCES
    src/factory.cpp
    src/systemd_creds_provider.cpp
)

if(LIBSECRET_FOUND)
    list(APPEND SECRET_PROVIDER_SOURCES src/libsecret_provider.cpp)
endif()

add_library(secret_provider STATIC ${SECRET_PROVIDER_SOURCES})
target_include_directories(secret_provider PUBLIC include)
target_compile_features(secret_provider PUBLIC cxx_std_23)
target_compile_options(secret_provider PRIVATE -Wno-missing-field-initializers -Wno-ignored-attributes)
target_link_libraries(secret_provider PUBLIC Boost::cobalt)

if(LIBSECRET_FOUND)
    target_compile_definitions(secret_provider PUBLIC SECRET_PROVIDER_HAS_LIBSECRET)
    target_include_directories(secret_provider PRIVATE ${LIBSECRET_INCLUDE_DIRS})
    target_link_libraries(secret_provider PRIVATE ${LIBSECRET_LIBRARIES})
    message(STATUS "secret_provider: libsecret support enabled")
else()
    message(STATUS "secret_provider: libsecret not found, only systemd-creds available")
endif()

# Tests — uses parent project's BINAPI2_WITH_TESTS flag
if(BINAPI2_WITH_TESTS)
    add_subdirectory(tests)
endif()
