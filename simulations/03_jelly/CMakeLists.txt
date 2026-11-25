add_executable(sim_03
    main.cpp
    JellyPhysics.hpp
    JellyPhysics.cpp
    JellyMeshGenerator.hpp
    JellyMeshGenerator.cpp
    JellySimApp.hpp
    JellySimApp.cpp
    JellySimUI.hpp
    JellySimUI.cpp
)

target_include_directories(sim_03
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sim_03
    PRIVATE
        core_lib
)

# Copy shaders to output directory so renderer can load them relative to exe
add_custom_command(
    TARGET sim_03 PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:sim_03>/res"
    COMMENT "Copying shaders and resources to output directory"
)
