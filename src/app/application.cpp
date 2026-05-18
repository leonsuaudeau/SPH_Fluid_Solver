#include "application.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdio>
#include <filesystem>
#include <glm/vec2.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include "scenes.h"
#include "scene_io.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Application::Application() :
    camera(glm::vec2(0, 0), 80, 0.1f),
    solver(0.9f, 1.1f, 20000, 0.1f, glm::vec2(0, -9.81f), 0.4f, 120.0f) {}

int Application::run() {
    std::filesystem::create_directories("../output");

    // Create GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    const float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    window = glfwCreateWindow(
        (int)(1280 * main_scale),
        (int)(800 * main_scale), "Fluid Solver", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // vsync

    if (!particle_renderer.init()) return 1;
    ImGuiLayer::init(window, main_scale);

    char input_buffer[256] = "";
    double current_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        // TODO: replace by mouse controls, mouse3 and dragging for move, scroll for zoom
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.move(0, 1);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.move(0, -1);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.move(-1, 0);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.move(1, 0);

        if (!state.paused) {
            solver.step();
            state.spigot_cooldown += solver.dt;
            if (state.spigot_enabled && state.spigot_cooldown > 0.05f) {
                state.spigot_cooldown = 0;
                solver.add_particle_grid({3,1}, {-48,60}, {1,0,0});
                solver.particles[solver.particles.size() - 1].vel = glm::vec2(0, -20);
                solver.particles[solver.particles.size() - 2].vel = glm::vec2(0, -20);
                solver.particles[solver.particles.size() - 3].vel = glm::vec2(0, -20);
            }
        }

        ImGuiLayer::beginFrame();
        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

        if (state.click_spawn_enabled && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
            double x, y;
            int display_w, display_h;
            glfwGetCursorPos(window, &x, &y);
            glfwGetFramebufferSize(window, &display_w, &display_h);
            const float ndcX = (2.0f * x) / display_w - 1.0f;
            const float ndcY = 1.0f - (2.0f * y) / display_h;
            const glm::mat4 inv_vp = glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix());
            glm::vec4 world_pos = inv_vp * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
            world_pos /= world_pos.w;

            solver.add_particle({world_pos.x, world_pos.y}, {1,0,0});
        }

        ImGui::Begin("Simulation control", nullptr, flags);
        if (ImGui::Button(state.paused ? "Resume" : "Pause")) {
            state.paused = !state.paused;
        }
        if (ImGui::Button("Scene 1")) {
            Scenes::load_scene_1(solver);
        }
        if (ImGui::Button("Scene 2")) {
            Scenes::load_scene_2(solver);
        }
        if (ImGui::Button("Scene 3")) {
            Scenes::load_scene_3(solver);
        }
        if (ImGui::Button("Scene 4")) {
            Scenes::load_scene_4(solver);
        }
        if (ImGui::Button("Add cubes")) {
            Scenes::add_cubes(solver);
        }
        if (ImGui::Button("Save custom scene")) {
            state.currently_typing = true;
        }
        if (state.currently_typing) {
            if (ImGui::InputText("Name", input_buffer, IM_ARRAYSIZE(input_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                std::string name = input_buffer;
                SceneIO::save_to_json(solver, name);
                state.currently_typing = false;
            }
        }

        if (ImGui::Button("Load custom scene")) {
            SceneIO::load_from_json(solver, "test");
        }
        if (ImGui::Button(state.spigot_enabled? "Turn off spigot" : "Turn on spigot")) {
            state.spigot_enabled = !state.spigot_enabled;
        }
        if (ImGui::Button(state.click_spawn_enabled? "Mouse spawn particle off" : "Mouse spawn particle on")) {
            state.click_spawn_enabled = !state.click_spawn_enabled;
        }
        ImGui::End();

        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Camera control", nullptr, flags);
        if (ImGui::Button("Reset")) {
            camera.setPosition(0, 0);
        }
        if (ImGui::Button(state.recording ? "Pause recording" : "Resume recording")) {
            state.recording = !state.recording;
        }
        ImGui::End();
        Ui::set_debug_overlay(state, solver, camera);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        camera.updateTransform(display_w, display_h);
        particle_renderer.render(solver, camera, display_w, display_h);
        screen_recorder.update(solver.dt, state.recording, display_w, display_h);

        ImGuiLayer::render();

        glfwSwapBuffers(window);

        state.frame_time = glfwGetTime() - current_time;
        current_time = glfwGetTime();
    }
    ImGuiLayer::shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
