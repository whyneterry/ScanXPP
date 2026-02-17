target_include_directories(${CMAKE_PROJECT_NAME}-exe
    PUBLIC
        # ${CMAKE_CURRENT_SOURCE_DIR}/boost/include/boost-1_90
        ${CMAKE_CURRENT_SOURCE_DIR}/asio/include/
)

target_include_directories(${CMAKE_PROJECT_NAME}-lib
    PUBLIC
        # ${CMAKE_CURRENT_SOURCE_DIR}/boost/include/boost-1_90
        ${CMAKE_CURRENT_SOURCE_DIR}/asio/include/
)