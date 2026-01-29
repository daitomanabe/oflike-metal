cmake_minimum_required(VERSION 3.15)
project(test15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(OFLIKE_METAL_LIB oflike-metal PATHS ${CMAKE_SOURCE_DIR}/../../build)

set(SOURCES
    src/Test15.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../../src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OFLIKE_METAL_LIB}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
