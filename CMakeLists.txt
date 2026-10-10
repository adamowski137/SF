cmake_minimum_required(VERSION 4.4)
project(Simulation VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. ADD THIRD-PARTY SUBMODULES ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/glm)
add_subdirectory(third_party/glew)

# --- 2. ADD CORE LIBRARY SUBDIRECTORY ---
add_subdirectory(core)

# --- 3. ADD SIMULATION APPLICATIONS ---
add_subdirectory(simulations/01_spring)

