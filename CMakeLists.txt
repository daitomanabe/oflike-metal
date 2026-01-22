cmake_minimum_required(VERSION 3.24)
project(oflike_metal_skeleton LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Third-party libraries (GLM, tess2, utf8)
add_subdirectory(third_party)

# Core source files
set(OFLIKE_CORE_SOURCES
  src/render/DrawList.cpp
  src/render/metal/MetalRenderer.mm
  src/render/metal/MetalTexture.mm
  src/render/metal/MetalImage.mm
  src/oflike/graphics/ofTrueTypeFont.mm
  src/oflike/3d/ofNode.cpp
  src/oflike/3d/ofCamera.cpp
  src/oflike/3d/ofEasyCam.cpp
  src/oflike/3d/of3dPrimitive.cpp
  src/oflike/3d/ofBoxPrimitive.cpp
  src/oflike/3d/ofSpherePrimitive.cpp
  src/oflike/3d/ofCylinderPrimitive.cpp
  src/oflike/3d/ofConePrimitive.cpp
  src/oflike/3d/ofPlanePrimitive.cpp
  src/oflike/3d/ofIcoSpherePrimitive.cpp
  src/oflike/lighting/ofLight.cpp
  src/oflike/lighting/ofMaterial.cpp
)

# Platform source files
set(OFLIKE_PLATFORM_SOURCES
  platform/macos/main.mm
  platform/macos/MacWindow.mm
)

# macOS Native Frameworks (Mac専用 - パフォーマンス最適化)
# NOTE: This project is Mac-only. All APIs are optimized for macOS.
# See ARCHITECTURE.md for the absolute policy on Mac-native APIs.
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
find_library(CORETEXT_FRAMEWORK CoreText REQUIRED)
find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
find_library(IMAGEIO_FRAMEWORK ImageIO REQUIRED)
find_library(ACCELERATE_FRAMEWORK Accelerate REQUIRED)

set(OFLIKE_FRAMEWORKS
  ${COCOA_FRAMEWORK}
  ${METAL_FRAMEWORK}
  ${METALKIT_FRAMEWORK}
  ${QUARTZCORE_FRAMEWORK}
  ${CORETEXT_FRAMEWORK}
  ${COREGRAPHICS_FRAMEWORK}
  ${IMAGEIO_FRAMEWORK}
  ${ACCELERATE_FRAMEWORK}
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
  )
  target_link_libraries(${name} PRIVATE
    ${OFLIKE_FRAMEWORKS}
    third_party
  )
endmacro()

# Examples
add_oflike_example(00_basic examples/00_basic/main.cpp)
add_oflike_example(01_particles examples/01_particles/main.cpp)
add_oflike_example(02_polylines examples/02_polylines/main.cpp)
add_oflike_example(03_image examples/03_image/main.cpp)
add_oflike_example(04_typography examples/04_typography/main.cpp)
add_oflike_example(05_3d_primitives examples/05_3d_primitives/main.cpp)
add_oflike_example(06_graphics examples/06_graphics/main.cpp)
add_oflike_example(07_color examples/07_color/main.cpp)
add_oflike_example(08_polygon examples/08_polygon/main.cpp)
add_oflike_example(09_lines examples/09_lines/main.cpp)
add_oflike_example(10_blending examples/10_blending/main.cpp)
