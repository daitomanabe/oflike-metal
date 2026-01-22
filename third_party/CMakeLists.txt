# Third-party libraries for oflike-metal
# See docs/THIRDPARTY.md for usage policy

# =============================================================================
# GLM - Header-only math library
# =============================================================================
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

# =============================================================================
# tess2 - Polygon tessellation library (precompiled xcframework)
# =============================================================================
add_library(tess2 INTERFACE)
target_include_directories(tess2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/tess2
)

# Link the xcframework for macOS
if(APPLE)
    set(TESS2_FRAMEWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tess2/tess2.xcframework")
    if(EXISTS "${TESS2_FRAMEWORK_PATH}")
        # Find the correct architecture folder in xcframework
        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set(TESS2_LIB_PATH "${TESS2_FRAMEWORK_PATH}/macos-arm64_x86_64/libtess2.a")
            if(NOT EXISTS "${TESS2_LIB_PATH}")
                # Try alternative path structure
                file(GLOB TESS2_LIB_PATHS "${TESS2_FRAMEWORK_PATH}/*/libtess2.a")
                if(TESS2_LIB_PATHS)
                    list(GET TESS2_LIB_PATHS 0 TESS2_LIB_PATH)
                endif()
            endif()
        endif()

        if(EXISTS "${TESS2_LIB_PATH}")
            target_link_libraries(tess2 INTERFACE "${TESS2_LIB_PATH}")
            message(STATUS "Found tess2 library: ${TESS2_LIB_PATH}")
        else()
            message(WARNING "tess2 library not found in xcframework")
        endif()
    endif()
endif()

# =============================================================================
# utf8 - Header-only UTF-8 string library
# =============================================================================
add_library(utf8 INTERFACE)
target_include_directories(utf8 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/utf8
)

# =============================================================================
# Convenience target for all third-party libraries
# =============================================================================
add_library(third_party INTERFACE)
target_link_libraries(third_party INTERFACE
    glm
    tess2
    utf8
)
