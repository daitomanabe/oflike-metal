# Third-party libraries for oflike-metal

# tess2 - Polygon tessellation library
file(GLOB TESS2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tess2/Source/*.c
)
add_library(tess2 STATIC ${TESS2_SOURCES})
target_include_directories(tess2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tess2/Include
)
set_target_properties(tess2 PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
)

# utf8-cpp - UTF-8 processing library
add_library(utf8 INTERFACE)
target_include_directories(utf8 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/utf8)
# Note: Header-only library

# oscpack - OSC (Open Sound Control) library
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/oscpack/osc/OscTypes.h)
    file(GLOB OSCPACK_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/oscpack/osc/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/oscpack/ip/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/oscpack/ip/posix/*.cpp
    )
    add_library(oscpack STATIC ${OSCPACK_SOURCES})
    target_include_directories(oscpack PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/oscpack
    )
    set_target_properties(oscpack PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
    message(STATUS "oscpack found (OSC protocol support)")
else()
    message(WARNING "oscpack not found - ofxOsc addon will not be available")
endif()

# GLM - OpenGL Mathematics (temporary, will be replaced by simd)
# Note: This is for compatibility during v0.1-v0.5
# Will be fully replaced by simd in v0.5.0+
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm/glm.hpp)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)
    message(STATUS "GLM found (temporary dependency, will be replaced by simd)")
else()
    message(WARNING "GLM not found - will use simd directly (recommended)")
endif()
