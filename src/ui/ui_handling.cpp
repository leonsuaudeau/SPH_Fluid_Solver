#include "ui_handling.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "implot.h"

void Ui::set_debug_overlay(const AppState &state, const FluidSolver &solver, const Camera2D &camera) {
    //ImGuiIO& io = ImGui::GetIO();
    constexpr float PAD = 10.0f;
    constexpr ImVec2 pos (PAD, PAD);
    constexpr ImVec2 pivot (0.0f, 0.0f);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
    ImGui::SetNextWindowSizeConstraints( ImVec2(260, 0), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowBgAlpha(0.35f); // transparent background

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("DebugOverlay", nullptr, flags)) {
        ImGui::Text("cfl: %.4f", solver.get_cfl_lambda());
        ImGui::Text("Particles: %d", solver.get_num_particles());
        ImGui::Separator();
        ImGui::Text("Delta time: %.3f ms (%.1f FPS)", state.frame_time * 1000.0f, 1.0f / state.frame_time);
        ImGui::Text("Camera:");
        glm::vec2 cam_pos = camera.getPosition();
        ImGui::Text("x: %.2f y: %.2f", cam_pos.x, cam_pos.y);
        if (state.selected_particle_index >= 0) {
            ImGui::Text("Selected particle: %d", state.selected_particle_index);
            ImGui::Text("Density: %f", solver.particles.rho[state.selected_particle_index]);
            ImGui::Text("Pressure: %f", solver.particles.p[state.selected_particle_index]);
            ImGui::Text("Mass: %f", solver.particles.m[state.selected_particle_index]);
        }
    }
    ImGui::End();
}

void ImGuiLayer::init(GLFWwindow *window, const float scale) {
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void ImGuiLayer::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::shutdown() {
    /// TODO: figure out reason for segfault (exit code 139)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
}
