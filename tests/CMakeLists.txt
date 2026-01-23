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
