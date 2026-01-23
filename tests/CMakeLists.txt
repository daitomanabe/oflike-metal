cmake_minimum_required(VERSION 3.20)

# Math Library Tests
add_executable(math_test
    math/math_test.cpp
)

target_include_directories(math_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/oflike/math
    ${CMAKE_SOURCE_DIR}/src/oflike/types
)

target_link_libraries(math_test PRIVATE
    oflike-metal
)

# Set C++ standard
set_target_properties(math_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Add test to CTest
enable_testing()
add_test(NAME MathLibrary COMMAND math_test)

# Rendering Tests
add_executable(rendering_test
    rendering/rendering_test.cpp
)

target_include_directories(rendering_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/oflike/app
    ${CMAKE_SOURCE_DIR}/src/oflike/graphics
    ${CMAKE_SOURCE_DIR}/src/oflike/image
    ${CMAKE_SOURCE_DIR}/src/oflike/3d
    ${CMAKE_SOURCE_DIR}/src/oflike/lighting
    ${CMAKE_SOURCE_DIR}/src/oflike/types
    ${CMAKE_SOURCE_DIR}/src/oflike/utils
)

target_link_libraries(rendering_test PRIVATE
    oflike-metal
)

# Set C++ standard
set_target_properties(rendering_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Add rendering test to CTest
add_test(NAME RenderingTests COMMAND rendering_test)

# Performance Tests
add_executable(performance_test
    performance/performance_test.cpp
)

target_include_directories(performance_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/oflike/app
    ${CMAKE_SOURCE_DIR}/src/oflike/graphics
    ${CMAKE_SOURCE_DIR}/src/oflike/image
    ${CMAKE_SOURCE_DIR}/src/oflike/3d
    ${CMAKE_SOURCE_DIR}/src/oflike/lighting
    ${CMAKE_SOURCE_DIR}/src/oflike/types
    ${CMAKE_SOURCE_DIR}/src/oflike/utils
    ${CMAKE_SOURCE_DIR}/src/oflike/math
)

target_link_libraries(performance_test PRIVATE
    oflike-metal
)

# Set C++ standard
set_target_properties(performance_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Add performance test to CTest
add_test(NAME PerformanceTests COMMAND performance_test)

# Batching Tests (Phase 18.1)
add_executable(batching_test
    test_batching.cpp
)

target_include_directories(batching_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(batching_test PRIVATE
    oflike-metal
)

# Set C++ standard
set_target_properties(batching_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Add batching test to CTest
add_test(NAME BatchingTests COMMAND batching_test)
