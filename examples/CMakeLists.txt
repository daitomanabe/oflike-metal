# Examples for oflike-metal

# Function to create an example app
function(add_oflike_example EXAMPLE_NAME)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/main.cpp)
        add_executable(${EXAMPLE_NAME}
            ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/main.cpp
        )

        target_link_libraries(${EXAMPLE_NAME} PRIVATE oflike-metal)

        # Copy resources if they exist
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/data)
            file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/data
                 DESTINATION ${CMAKE_BINARY_DIR}/examples/${EXAMPLE_NAME})
        endif()

        message(STATUS "Added example: ${EXAMPLE_NAME}")
    endif()
endfunction()

# Add examples as they are created
add_oflike_example(01_basics)
add_oflike_example(02_shapes)
# add_oflike_example(03_color)
# etc.

# Standalone test programs (not full apps)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_japanese_font.cpp)
    add_executable(test_japanese_font
        ${CMAKE_CURRENT_SOURCE_DIR}/test_japanese_font.cpp
    )
    target_link_libraries(test_japanese_font PRIVATE oflike-metal)
    message(STATUS "Added test: test_japanese_font")
endif()
