cmake_minimum_required(VERSION 3.15)
project(test14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the oflike-metal library
find_library(OFLIKE_METAL_LIB oflike-metal PATHS ${CMAKE_SOURCE_DIR}/../../build)

# Source files
set(SOURCES
    src/Test14.cpp
)

# Create shared library (plugin)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../../src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OFLIKE_METAL_LIB}
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
