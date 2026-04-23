cmake_minimum_required(VERSION 4.2)
project(SPH_Fluid_Solver)

set(CMAKE_CXX_STANDARD 26)

find_package(glm CONFIG REQUIRED)

add_executable(SPH_Fluid_Solver src/main.cpp
        include/particle.h)

target_include_directories(SPH_Fluid_Solver PRIVATE include)
target_link_libraries(SPH_Fluid_Solver PRIVATE glm::glm)
