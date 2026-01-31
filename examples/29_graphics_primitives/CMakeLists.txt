cmake_minimum_required(VERSION 3.20)
project(test13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find oflike-metal framework
find_package(oflike-metal REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.mm")
file(GLOB_RECURSE HEADERS "src/*.h")

# Swift sources
file(GLOB SWIFT_SOURCES "src/*.swift")

# Executable
add_executable(test13 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${SWIFT_SOURCES})

# Link oflike-metal
target_link_libraries(test13 oflike-metal::oflike-metal)

# Bundle settings
set_target_properties(test13 PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist"
    RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources;${CMAKE_CURRENT_SOURCE_DIR}/data"
)
