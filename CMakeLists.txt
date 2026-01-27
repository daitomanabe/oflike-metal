cmake_minimum_required(VERSION 3.20)
project(oflike-metal VERSION 0.1.0 LANGUAGES C CXX OBJCXX)

# Project settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Swift support (requires Xcode generator)
# Use: cmake -G Xcode .. to enable Swift
if(CMAKE_Swift_COMPILER)
    enable_language(Swift)
    set(CMAKE_Swift_LANGUAGE_VERSION 5)
endif()

# macOS deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS deployment version")

# Optional frameworks (feature-gated)
option(OFLIKE_ENABLE_VISIONKIT "Enable VisionKit (macOS 14+, optional)" OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Enable Objective-C ARC for both C++ and Objective-C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} -fobjc-arc")

# Find required frameworks
find_library(COCOA_LIBRARY Cocoa REQUIRED)
find_library(METAL_LIBRARY Metal REQUIRED)
find_library(METALKIT_LIBRARY MetalKit REQUIRED)
find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)
find_library(CORETEXT_LIBRARY CoreText REQUIRED)
find_library(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)
find_library(IMAGEIO_LIBRARY ImageIO REQUIRED)
find_library(ACCELERATE_LIBRARY Accelerate REQUIRED)

if(OFLIKE_ENABLE_VISIONKIT)
    find_library(VISIONKIT_LIBRARY VisionKit)
    if(NOT VISIONKIT_LIBRARY)
        message(FATAL_ERROR "VisionKit framework not found (requires macOS 14 SDK).")
    endif()
    if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "14.0")
        message(WARNING "VisionKit is macOS 14+; deployment target is ${CMAKE_OSX_DEPLOYMENT_TARGET}.")
    endif()
endif()

# Third-party libraries
add_subdirectory(third_party)

# Main library sources
file(GLOB_RECURSE CORE_SOURCES
    src/core/*.cpp
    src/core/*.mm
)

file(GLOB_RECURSE MATH_SOURCES
    src/math/*.cpp
    src/math/*.mm
)

file(GLOB_RECURSE PLATFORM_SOURCES
    src/platform/*.cpp
    src/platform/*.mm
)

# Add Swift sources only if Swift compiler is available
if(CMAKE_Swift_COMPILER)
    file(GLOB_RECURSE PLATFORM_SWIFT_SOURCES
        src/platform/*.swift
    )
    list(APPEND PLATFORM_SOURCES ${PLATFORM_SWIFT_SOURCES})
    message(STATUS "Swift compiler found - including Swift sources")
else()
    message(STATUS "Swift compiler not found - skipping Swift sources (use 'cmake -G Xcode' to enable)")
endif()

file(GLOB_RECURSE RENDER_SOURCES
    src/render/*.cpp
    src/render/*.mm
)

file(GLOB_RECURSE OFLIKE_SOURCES
    src/oflike/*.cpp
    src/oflike/*.mm
)

# Core addons only (excluding apple_native/ofxSharp which has incomplete implementation)
file(GLOB_RECURSE ADDON_CORE_SOURCES
    src/addons/core/**/*.cpp
    src/addons/core/**/*.mm
)
set(ADDON_SOURCES ${ADDON_CORE_SOURCES})

# Main oflike-metal library
# Note: Create a dummy source file to allow library creation before implementation
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${MATH_SOURCES}
    ${PLATFORM_SOURCES}
    ${RENDER_SOURCES}
    ${OFLIKE_SOURCES}
    ${ADDON_SOURCES}
)

if(ALL_SOURCES)
    add_library(oflike-metal STATIC ${ALL_SOURCES})
else()
    # Create empty library for now - will be populated as implementation progresses
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "// Placeholder\n")
    add_library(oflike-metal STATIC ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
    message(STATUS "Creating oflike-metal with placeholder (no source files yet)")
endif()

target_include_directories(oflike-metal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render
    ${CMAKE_CURRENT_SOURCE_DIR}/src/oflike
    ${CMAKE_CURRENT_SOURCE_DIR}/src/addons/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/addons/apple_native
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(oflike-metal PUBLIC
    ${COCOA_LIBRARY}
    ${METAL_LIBRARY}
    ${METALKIT_LIBRARY}
    ${QUARTZCORE_LIBRARY}
    ${CORETEXT_LIBRARY}
    ${COREGRAPHICS_LIBRARY}
    ${IMAGEIO_LIBRARY}
    ${ACCELERATE_LIBRARY}
    tess2
    utf8
)

if(OFLIKE_ENABLE_VISIONKIT)
    target_link_libraries(oflike-metal PUBLIC ${VISIONKIT_LIBRARY})
endif()

# Link nanosvg if available
if(TARGET nanosvg)
    target_link_libraries(oflike-metal PUBLIC nanosvg)
endif()

# Link pugixml if available
if(TARGET pugixml)
    target_link_libraries(oflike-metal PUBLIC pugixml)
endif()

# Link oscpack if available
if(TARGET oscpack)
    target_link_libraries(oflike-metal PUBLIC oscpack)
endif()

# Copy shaders to build directory
file(GLOB SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.metal)
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()

# Examples are pending SwiftUI migration (not built by default)

# Example Launcher (Swift GUI tool)
add_custom_target(launcher
    COMMAND swiftc -o ${CMAKE_BINARY_DIR}/tools/ExampleLauncher/ExampleLauncher
            -framework AppKit
            ${CMAKE_SOURCE_DIR}/tools/ExampleLauncher/ExampleLauncher.swift
    COMMAND ${CMAKE_BINARY_DIR}/tools/ExampleLauncher/ExampleLauncher
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building and launching Example Launcher"
)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tools/ExampleLauncher)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS oflike-metal
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY src/oflike/
    DESTINATION include/oflike
    FILES_MATCHING PATTERN "*.h"
)

# Print configuration
message(STATUS "oflike-metal configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  macOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
