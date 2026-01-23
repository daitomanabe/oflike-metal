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
# add_oflike_example(00_basic)
# add_oflike_example(01_particles)
# add_oflike_example(02_polylines)
# etc.
