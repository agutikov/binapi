add_library(glaze_glaze INTERFACE)
add_library(glaze::glaze ALIAS glaze_glaze)

target_include_directories(glaze_glaze
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/glaze/include
)

target_compile_features(glaze_glaze INTERFACE cxx_std_23)

# CLI11 — header-only command-line parser.
add_library(CLI11 INTERFACE)
add_library(CLI11::CLI11 ALIAS CLI11)
target_include_directories(CLI11 SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/CLI11/include
)
target_compile_features(CLI11 INTERFACE cxx_std_17)

# FTXUI — terminal UI library.  Built as a subdirectory; we disable its
# examples/tests/docs to keep the configure step fast.
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(FTXUI EXCLUDE_FROM_ALL SYSTEM)
