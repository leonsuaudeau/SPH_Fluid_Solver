add_library(sph_solver
        sph/solver.cpp
        sph/sph_kernel.h
        sph/sph_integrators.h
)

target_include_directories(sph_solver PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/sph
)

target_link_libraries(sph_solver PUBLIC glm::glm)

add_executable(SPH_Fluid_Solver
        main.cpp
        camera.cpp
        app/application.cpp
        app/scenes.cpp
        rendering/particle_renderer.cpp
        rendering/shader_utilities.cpp
        rendering/screen_recorder.cpp
        ui/ui_handling.cpp

        ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/misc/cpp/imgui_stdlib.cpp
)

target_compile_definitions(SPH_Fluid_Solver PRIVATE
        SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders/"
)

target_include_directories(SPH_Fluid_Solver PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/imgui
        ${CMAKE_SOURCE_DIR}/external/imgui/backends
        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/rendering
        ${CMAKE_SOURCE_DIR}/src/ui
)

target_compile_definitions(SPH_Fluid_Solver PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_link_libraries(SPH_Fluid_Solver PRIVATE
        sph_solver
        glm::glm
        OpenGL::GL
        glfw
        glad
)