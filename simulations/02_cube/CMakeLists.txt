add_executable(sim_02
    main.cpp
    CubeSimApp.cpp
    CubePhysics.cpp
    CubeMeshGenerator.cpp
    CubeSimUI.cpp
)

target_include_directories(sim_02
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sim_02
    PRIVATE 
        core_lib
)

# Copy shaders to output directory so renderer can load them relative to exe
add_custom_command(
    TARGET sim_02 PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:sim_02>/res"
    COMMENT "Copying shaders and resources to output directory"
)
