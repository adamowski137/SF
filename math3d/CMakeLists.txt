project(math3d)

add_library(math3d INTERFACE "vector/vector.hpp" "matrix/matrix.hpp" "quat/quat.hpp")

target_include_directories(math3d INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
