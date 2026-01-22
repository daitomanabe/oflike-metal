cmake_minimum_required(VERSION 3.24)
project(oflike_metal_skeleton LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(basic_app MACOSX_BUNDLE
  platform/macos/main.mm
  src/render/Renderer2D.mm
  examples/basic_app.cpp
)

target_include_directories(basic_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Frameworks
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)

target_link_libraries(basic_app PRIVATE
  ${COCOA_FRAMEWORK}
  ${METAL_FRAMEWORK}
  ${METALKIT_FRAMEWORK}
  ${QUARTZCORE_FRAMEWORK}
)

# TODO: add Metal shader compilation + bundling when Renderer2D uses MSL pipeline.
