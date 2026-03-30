add_executable(
    ${PROJECT_NAME}_app
    main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_app
    PRIVATE
        ${PROJECT_NAME}
)

add_executable(binapi_synchronous synchronous/main.cpp)
target_link_libraries(binapi_synchronous PRIVATE ${PROJECT_NAME})

add_executable(binapi_asynchronous asynchronous/main.cpp)
target_link_libraries(binapi_asynchronous PRIVATE ${PROJECT_NAME})

add_executable(binapi_synchronous_user_data synchronous-user_data/main.cpp)
target_link_libraries(binapi_synchronous_user_data PRIVATE ${PROJECT_NAME})

add_executable(binapi_asynchronous_user_data asynchronous-user_data/main.cpp)
target_link_libraries(binapi_asynchronous_user_data PRIVATE ${PROJECT_NAME})

add_executable(binapi_websockets websockets/main.cpp)
target_link_libraries(binapi_websockets PRIVATE ${PROJECT_NAME})
