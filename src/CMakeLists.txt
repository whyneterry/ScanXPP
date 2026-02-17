add_library(${CMAKE_PROJECT_NAME}-lib)

target_include_directories(${CMAKE_PROJECT_NAME}-lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${CMAKE_PROJECT_NAME}-lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/header.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/mtportscanner.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/timeanalyzer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/progressbar.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/scansession.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/tsvector.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/tsmap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/scanresultstruct.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/header.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/mtportscanner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/timeanalyzer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/progressbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/scansession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/tsvector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/scanresultstruct.cpp
)

add_executable(${CMAKE_PROJECT_NAME}-exe)

target_link_libraries(${CMAKE_PROJECT_NAME}-exe
    ${CMAKE_PROJECT_NAME}-lib        
)

target_sources(${CMAKE_PROJECT_NAME}-exe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app/main.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}-exe
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)