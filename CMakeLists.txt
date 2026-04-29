cmake_minimum_required(VERSION 4.2)
project(SPH_Fluid_Solver)

set(CMAKE_CXX_STANDARD 26)

find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(glad external/glad/src/glad.c
        src/sph/solver.cpp
        src/sph/solver.h)
target_include_directories(glad PUBLIC external/glad/include)
add_subdirectory(src)

option(ENABLE_TESTS "Build tests" ON)
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
