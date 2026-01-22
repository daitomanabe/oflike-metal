cmake_minimum_required(VERSION 3.24)
project(oflike_metal_skeleton LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core source files
set(OFLIKE_CORE_SOURCES
  src/render/DrawList.cpp
  src/render/metal/MetalRenderer.mm
  src/render/metal/MetalTexture.mm
  src/render/metal/MetalImage.mm
)

# Platform source files
set(OFLIKE_PLATFORM_SOURCES
  platform/macos/main.mm
  platform/macos/MacWindow.mm
)

# Frameworks
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)

set(OFLIKE_FRAMEWORKS
  ${COCOA_FRAMEWORK}
  ${METAL_FRAMEWORK}
  ${METALKIT_FRAMEWORK}
  ${QUARTZCORE_FRAMEWORK}
)

# Macro to add an example
macro(add_oflike_example name source)
  add_executable(${name} MACOSX_BUNDLE
    ${OFLIKE_CORE_SOURCES}
    ${OFLIKE_PLATFORM_SOURCES}
    ${source}
  )
  target_include_directories(${name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
  )
  target_link_libraries(${name} PRIVATE ${OFLIKE_FRAMEWORKS})
endmacro()

# Examples
add_oflike_example(00_basic examples/00_basic/main.cpp)
add_oflike_example(01_particles examples/01_particles/main.cpp)
add_oflike_example(02_polylines examples/02_polylines/main.cpp)
add_oflike_example(03_image examples/03_image/main.cpp)
add_oflike_example(04_typography examples/04_typography/main.cpp)
add_oflike_example(05_3d_primitives examples/05_3d_primitives/main.cpp)
