add_executable(sim_01
    main.cpp
    MathParser.hpp
    MathParser.cpp
    SpringPhysics.hpp
    SpringPhysics.cpp
    PlotWidget.hpp
    PlotWidget.cpp
    SpringMeshGenerator.hpp
    SpringMeshGenerator.cpp
    SpringSimApp.hpp
    SpringSimApp.cpp
    SpringSimUI.hpp
    SpringSimUI.cpp
)

target_include_directories(sim_01
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sim_01
    PRIVATE 
        core_lib
)

# Copy shaders to output directory so renderer can load them relative to exe
add_custom_command(
    TARGET sim_01 PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:sim_01>/res"
    COMMENT "Copying shaders and resources to output directory"
)
