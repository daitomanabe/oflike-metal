# Examples for oflike-metal

# Function to create an example app
function(add_oflike_example EXAMPLE_NAME)
    # Check for .mm (Objective-C++) or .cpp file
    set(EXAMPLE_SOURCE "")
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/main.mm)
        set(EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/main.mm)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/main.cpp)
        set(EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/main.cpp)
    endif()

    if(EXAMPLE_SOURCE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})

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
add_oflike_example(03_color)
add_oflike_example(04_image)
add_oflike_example(05_typography)
add_oflike_example(06_3d_primitives)
add_oflike_example(07_camera)
add_oflike_example(08_lighting)
add_oflike_example(09_mesh)
add_oflike_example(10_fbo)
add_oflike_example(11_events)
add_oflike_example(12_easycam)
add_oflike_example(13_osc_sender)
add_oflike_example(14_osc_receiver)
add_oflike_example(15_gui_basic)
add_oflike_example(16_gui_groups)
add_oflike_example(16_xml_settings)
add_oflike_example(17_svg_loader)
add_oflike_example(17_tcp_client)
add_oflike_example(18_tcp_server)
add_oflike_example(19_osc_parameter_sync)
add_oflike_example(19_udp_sender)
add_oflike_example(20_udp_receiver)
add_oflike_example(21_face_detection)
add_oflike_example(22_sharp_basic)
add_oflike_example(23_sharp_scene)
add_oflike_example(24_sharp_video_export)
add_oflike_example(25_sharp_realtime_camera)
# etc.

# Standalone test programs (not full apps)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_japanese_font.cpp)
    add_executable(test_japanese_font
        ${CMAKE_CURRENT_SOURCE_DIR}/test_japanese_font.cpp
    )
    target_link_libraries(test_japanese_font PRIVATE oflike-metal)
    message(STATUS "Added test: test_japanese_font")
endif()
