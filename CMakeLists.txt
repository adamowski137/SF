cmake_minimum_required(VERSION 3.20)
project(Simulation VERSION 1.0.0 LANGUAGES CXX)

if (MSVC)
    set(CMAKE_CXX26_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX26_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

set(CMAKE_CXX_STANDARD 20)
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

# --- 4. ADD SIMULATION APPLICATIONS ---
add_subdirectory(simulations/01_spring)
