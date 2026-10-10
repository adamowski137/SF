# core/CMakeLists.txt

add_library(core_lib STATIC
  src/Window.cpp
)

# Expose public headers for core and dependent targets
target_include_directories(core_lib
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link external libraries (GLM exposed publicly because simulations likely need math headers)
target_link_libraries(core_lib
    PUBLIC
        glm::glm
    PRIVATE
        glfw
        libglew_static
)
