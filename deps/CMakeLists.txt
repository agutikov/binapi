add_library(glaze_glaze INTERFACE)
add_library(glaze::glaze ALIAS glaze_glaze)

target_include_directories(glaze_glaze
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/glaze/include
)

target_compile_features(glaze_glaze INTERFACE cxx_std_23)
