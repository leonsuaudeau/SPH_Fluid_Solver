cmake_minimum_required(VERSION 4.2)
project(SPH_Fluid_Solver)

set(CMAKE_CXX_STANDARD 26)

find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

file(CREATE_LINK
        ${CMAKE_SOURCE_DIR}/shaders
        ${CMAKE_BINARY_DIR}/shaders
        SYMBOLIC
)

add_executable(SPH_Fluid_Solver
        src/main.cpp
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        external/imgui/misc/cpp/imgui_stdlib.cpp
        src/camera.cpp
        src/shader_utilities.cpp
)

target_compile_definitions(SPH_Fluid_Solver PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)


add_library(glad external/glad/src/glad.c
        include/motion_update.h)
target_include_directories(glad PUBLIC external/glad/include)

target_include_directories(SPH_Fluid_Solver PRIVATE include external/imgui external/imgui/backends)
target_link_libraries(SPH_Fluid_Solver PRIVATE glm::glm OpenGL::GL glfw glad)
