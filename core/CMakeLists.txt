# core/CMakeLists.txt

set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/third_party/imgui")

file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(core_lib STATIC
  src/error.cpp
  src/Mesh.cpp
  src/MeshGenerator.cpp
  src/Shader.cpp
  src/ShaderBuilder.cpp
  src/ShaderManager.cpp
  src/Camera.cpp
  src/Window.cpp
  src/InputHandler.cpp
  src/UI.cpp
  src/Renderer.cpp
  src/App.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(core_lib
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(core_lib
    PUBLIC
        glm::glm
        math3d
        glfw
        libglew_static
        opengl32
)
