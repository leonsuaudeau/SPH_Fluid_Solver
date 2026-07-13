#include "application.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <filesystem>
#include <glm/vec2.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <glm/ext/matrix_transform.hpp>

#include "glfw_user_pointer.h"
#include "scenes.h"
#include "scene_io.h"

#include <implot.h>

#ifdef _OPENMP
#include <omp.h>
#endif


constexpr ImGuiWindowFlags global_flags = ImGuiWindowFlags_AlwaysAutoResize;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    const GLFWUserPointer *user_pointer = static_cast<GLFWUserPointer*> (glfwGetWindowUserPointer(window));
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        if (user_pointer->state.placement_tool_active) {
            user_pointer->state.rect_r += glm::radians(-glm::sign(yoffset));
        }
    }else {
        user_pointer->camera.zoom(-yoffset);
    }
}

void print_openmp_info() {
    #ifdef _OPENMP
    std::cout << "OpenMP enabled\n";
    std::cout << "_OPENMP version: " << _OPENMP << "\n";
    std::cout << "Max threads: " << omp_get_max_threads() << "\n";

    omp_set_dynamic(0);
    omp_set_num_threads(6);

    #pragma omp parallel
    {
    #pragma omp single
        {
            std::cout << "Actual threads in parallel region: "
                      << omp_get_num_threads() << "\n";
        }
    }
    #else
    std::cout << "OpenMP NOT enabled at compile time\n";
    #endif
    }

Application::Application() :
    camera(glm::vec2(0, 0), 80, 0.1f),
    solver(0.001f, 0.9f, 1.1f, 20000, 0.5f, glm::vec2(0, -9.81f), 1.0f, 0.9f, 200.0f, 1000.0f),
    grid(512, 512, {-256, -256}, solver.h, solver.particles){

    // Load files in scene path once
    scenes = SceneIO::get_scene_entries("scenes/");
    snapshots = SceneIO::get_scene_entries("snapshots/");
    std::filesystem::create_directories("../../output");

    stat_seq.density_error[0] = 0;
    stat_seq.density_error_no_surface[0] = 0;
    stat_seq.time[0] = 0;

    print_openmp_info();
}

int Application::run() {
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

    glfwSetScrollCallback(window, scroll_callback);
    GLFWUserPointer user_pointer(camera, state);
    glfwSetWindowUserPointer(window, &user_pointer);

    if (!particle_renderer.init()) return 1;
    if (!grid_renderer.init()) return 1;
    ImGuiLayer::init(window, main_scale);

    double current_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // TODO: replace by mouse controls, mouse3 and dragging for move, scroll for zoom
        if (!state.currently_typing) {
            // THIS IS ABSOLUTELY HORRIBLE!!!!!!
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !state.space_last_pressed && state.app_mode != edit_scene) {
                state.paused = !state.paused;
                state.space_last_pressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !state.tab_last_pressed) {
                state.app_mode = (state.app_mode == simulate) ? edit_scene : simulate;
                state.paused = true;
                state.selected_particle_index = -1;
                state.tab_last_pressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && state.space_last_pressed) {
                state.space_last_pressed = false;
            }
            if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && state.tab_last_pressed) {
                state.tab_last_pressed = false;
            }

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.move(0, 1);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.move(0, -1);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.move(-1, 0);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.move(1, 0);
        }else {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) state.currently_typing = false;
        }

        if (!state.paused) {
            solver.step(grid);
            state.spigot_cooldown += solver.dt;
            if (state.spigot_enabled && state.spigot_cooldown > 0.05f) {
                state.spigot_cooldown = 0;
                solver.add_particle_grid({3,1}, {-48,60}, {1,0,0});
                solver.particles.v_y[solver.particles.count - 1] = -20;
                solver.particles.v_y[solver.particles.count - 2] = -20;
                solver.particles.v_y[solver.particles.count - 3] = -20;
            }
        }

        ImGuiLayer::beginFrame();
        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::Begin("Mode select", nullptr, global_flags);
        if (ImGui::Button("Simulate")) {
            state.app_mode = simulate;
        }
        if (ImGui::Button("Edit Scene")) {
            state.app_mode = edit_scene;
            state.paused = true;
            state.selected_particle_index = -1;
        }
        if (ImGui::Button("View Statistics")) {
            state.app_mode = view_plot;
            state.paused = true;
            state.selected_particle_index = -1;
        }
        ImGui::End();

        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Camera control", nullptr, global_flags);
        if (ImGui::Button("Reset")) {
            camera.setPosition(0, 0);
        }
        if (ImGui::Button(state.recording ? "Pause recording" : "Resume recording")) {
            state.recording = !state.recording;
        }
        ImGui::End();

        switch (state.app_mode) {
            case simulate:
                ui_simulate();
                break;
            case edit_scene:
                ui_scene_editor();
                break;
            case view_plot:
                ui_view_plot();
                break;
        }

        Ui::set_debug_overlay(state, solver, camera);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        camera.updateTransform(display_w, display_h);


        // Rendering
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        grid_renderer.render(grid, camera);
        particle_renderer.render(solver, state, camera);

        if (state.app_mode == edit_scene) {
            particle_renderer.render(preview_particles, solver.h / 2.0f, camera);
        }
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

void Application::ui_simulate() {
    ImGui::SetNextWindowSizeConstraints( ImVec2(300, 0), ImVec2(FLT_MAX, FLT_MAX));

    // handle particle selection
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
        const glm::vec2 mouse_pos = camera.get_cursor_world_pos(window);
        float min_dist = 100;
        for (int offset_x = -1; offset_x < 2; offset_x++) {
            for (int offset_y = -1; offset_y < 2; offset_y++) {
                const float p_x_o = mouse_pos.x + offset_x * grid.cell_size;
                const float p_y_o = mouse_pos.y + offset_y * grid.cell_size;

                const int cell = grid.get_cell_index(p_x_o, p_y_o);

                if (cell == -1) continue;

                for (int k = 0; k < grid.counts[cell]; k++) {
                    int i = grid.particle_indices[cell * MAX_PARTICLES_PER_CELL + k];
                    const float dist = glm::length(mouse_pos - glm::vec2(grid.particles.p_x[i], grid.particles.p_y[i]));
                    if (dist < min_dist) {
                        min_dist = dist;
                        state.selected_particle_index = i;
                    }
                }
            }
        }
        if (min_dist > 99) state.selected_particle_index = -1;
    }

    ImGui::Begin("Simulation control", nullptr, global_flags);
    if (ImGui::Button(state.paused ? "Resume" : "Pause")) {
        state.paused = !state.paused;
    }
    /*
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
    */
    /* Snapshots */
    ImGui::TextColored(ImVec4(1,1,0,1), "Snapshots");
    if (ImGui::Button("Save snapshot")) {
        const time_t timestamp = time(nullptr);
        const tm datetime = *localtime(&timestamp);
        char output[50];
        strftime(output, 50, "%y:%m:%d:%I:%M:%S", &datetime);

        const std::string name = "snapshot " + static_cast<std::string>(output);
        SceneIO::save_to_json(solver, name, "snapshots/");
        snapshots = SceneIO::get_scene_entries("snapshots/");
    }
    ImGui::BeginChild("Scrolling", ImVec2(280, 80), ImGuiChildFlags_None);
    int id = 0;
    for (auto &entry: snapshots) {
        ImGui::PushID(id++);
        ImGui::TextColored(ImVec4(0,1,0,1), entry.c_str());
        ImGui::SameLine();
        if (ImGui::Button("load")) {
            SceneIO::load_from_json(solver, entry, "snapshots/");
            grid.set_cell_size(solver.h);
            solver.update_nu_boundaries(grid);
        }
        ImGui::SameLine();
        if (ImGui::Button("X")) {
            SceneIO::remove_scene_entry(entry, "snapshots/");
            snapshots = SceneIO::get_scene_entries("snapshots/");
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::TextColored(ImVec4(1,1,0,1), "Set parameters");
    ImGui::InputFloat("dt", &solver.dt);
    bool update_boundary_masses = false;
    float h_input = solver.h;
    if (ImGui::InputFloat("h", &h_input) && h_input > 0.0f) {
        solver.h = h_input;
        grid.set_cell_size(solver.h);
        update_boundary_masses = true;
    }
    if (ImGui::InputFloat("rho_0", &solver.rho_0)) {
        update_boundary_masses = true;
    }
    if (ImGui::InputFloat("gamma_1", &solver.gamma_1)) {
        update_boundary_masses = true;
    }
    ImGui::InputFloat("gamma_2", &solver.gamma_2);
    ImGui::InputFloat("gamma_st", &solver.gamma_st);
    ImGui::InputFloat("gamma_ad", &solver.gamma_ad);
    if (update_boundary_masses) {
        solver.update_nu_boundaries(grid);
    }
    ImGui::InputFloat("k", &solver.k);
    ImGui::InputFloat("nu", &solver.nu);

    if (ImGui::Button("Live statistics")) {
        state.plot_enabled = !state.plot_enabled;
    }

    if (ImGui::Button("Add cubes")) {
        Scenes::add_cubes(solver);
    }
    if (ImGui::Button(state.spigot_enabled? "Turn off spigot" : "Turn on spigot")) {
        state.spigot_enabled = !state.spigot_enabled;
    }
    ImGui::Checkbox("Highlight neighbors", &state.draw_neighbors);
    ImGui::End();

    if (state.plot_enabled) {
        // Update plot values
        if (!state.paused && state.stat_seq_index < stats::MAX_MEMORY - 1) {
            float rho_avg = 0;
            float rho_avg_no_surface = 0;

            for (int i = 0; i < solver.get_num_particles(); i++) {
                if (solver.particles.is_bound[i]) continue;
                rho_avg += solver.particles.rho[i] - solver.rho_0;

                if (solver.particles.rho[i] < solver.rho_0) continue;
                rho_avg_no_surface += solver.particles.rho[i] - solver.rho_0;
            }

            rho_avg /= static_cast<float>(solver.get_num_particles());
            rho_avg_no_surface /= static_cast<float>(solver.get_num_particles());

            stat_seq.density_error[state.stat_seq_index] = rho_avg;
            stat_seq.density_error_no_surface[state.stat_seq_index] = rho_avg_no_surface;

            stat_seq.time[state.stat_seq_index] = state.stat_seq_time;
            state.stat_seq_index++;
            state.stat_seq_time += solver.dt;
        }

        // Show plot
        ImGui::SetNextWindowSizeConstraints( ImVec2(600, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Live statistics", &state.plot_enabled);
        char info_text[1024];
        sprintf(info_text, "%d / %d datapoints", state.stat_seq_index, stats::MAX_MEMORY - 1);
        ImGui::TextColored(ImVec4(1,1,0,1), info_text);
        if (state.stat_seq_index >= stats::MAX_MEMORY - 1) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1,0,0,1), "MAXIMUM PLOT LENGTH, PLEASE RESET");
        }
        if (ImPlot::BeginPlot("Plot")) {
            if (state.move_with_plot) {
                const double width = state.x_max - state.x_min;
                state.x_min = state.stat_seq_time - width / 2;
                state.x_max = state.stat_seq_time + width / 2;
            }
            ImPlot::SetupAxisLinks(ImAxis_X1, &state.x_min, &state.x_max);
            ImPlot::SetupAxisLinks(ImAxis_Y1, &state.y_min, &state.y_max);
            //ImPlot::PlotInfLines("Rest density", &solver.rho_0, 1, {ImPlotProp_Flags, ImPlotInfLinesFlags_Horizontal});
            ImPlot::PlotLine("Average density error", stat_seq.time, stat_seq.density_error, state.stat_seq_index);
            ImPlot::PlotLine("Average density error interior", stat_seq.time, stat_seq.density_error_no_surface, state.stat_seq_index);
            ImPlot::EndPlot();
        }
        ImGui::Checkbox("Follow plot", &state.move_with_plot);
        ImGui::SameLine();
        if (ImGui::Button("Reset plot")) {
            const double width = state.x_max - state.x_min;
            state.x_min = -1.0;
            state.x_max = width - 1;
            stat_seq.density_error[0] = 0;
            stat_seq.density_error_no_surface[0] = 0;
            stat_seq.time[0] = 0;
            state.stat_seq_index = 0;
            state.stat_seq_time = 0;
        }

        // Storing statistics
        if (ImGui::Button("Save statistics")) {
            state.currently_typing = true;
        }
        if (state.currently_typing) {
            if (ImGui::InputText("Name", state.input_buffer0, 24, ImGuiInputTextFlags_EnterReturnsTrue)) {
                const std::string name = state.input_buffer0;
                // TODO: add option for having different values for both axes
                stats::save_to_file(name, stat_seq.density_error_no_surface, stat_seq.time, state.stat_seq_index);
                state.currently_typing = false;
            }
        }

        ImGui::End();
    }
}

void Application::ui_scene_editor() {
    ImGui::Begin("Editor control", nullptr, global_flags);

    ImGui::TextColored(ImVec4(1,1,0,1), "Scenes");
    if (ImGui::Button("Save custom scene")) {
        state.currently_typing = true;
    }
    if (state.currently_typing) {
        if (ImGui::InputText("Name", state.input_buffer0, 24, ImGuiInputTextFlags_EnterReturnsTrue)) {
            const std::string name = state.input_buffer0;
            SceneIO::save_to_json(solver, name, "scenes/");
            scenes = SceneIO::get_scene_entries("scenes/");
            state.currently_typing = false;
        }
    }

    ImGui::BeginChild("Scrolling", ImVec2(280, 140), ImGuiChildFlags_None);
    int id = 0;
    for (auto &entry: scenes) {
        ImGui::PushID(id++);
        ImGui::TextColored(ImVec4(0,1,0,1), entry.c_str());
        ImGui::SameLine();
        if (ImGui::Button("load")) {
            SceneIO::load_from_json(solver, entry, "scenes/");
            grid.set_cell_size(solver.h);
            solver.update_nu_boundaries(grid);
        }
        ImGui::SameLine();
        if (ImGui::Button("X")) {
            SceneIO::remove_scene_entry(entry, "scenes/");
            scenes = SceneIO::get_scene_entries("scenes/");
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    if (ImGui::Button("Placement tool")) {
        state.placement_tool_active = !state.placement_tool_active;
    }
    if (ImGui::Button("Draw grid")) {
        state.grid_edit_mode = !state.grid_edit_mode;
    }
    ImGui::End();

    if (state.grid_edit_mode) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
            if (!state.dragging_grid) {
                grid.origin = camera.get_cursor_world_pos(window);
                state.dragging_grid = true;
            }else {
                state.dragging_grid = false;
                state.grid_edit_mode = false;
            }
        }else {
            if (state.dragging_grid) {
                const glm::vec2 mouse_pos = camera.get_cursor_world_pos(window);
                const glm::vec2 local_pos = mouse_pos - grid.origin;
                const int width = static_cast<int>(std::floor(local_pos.x / grid.cell_size));
                const int height = static_cast<int>(std::floor(local_pos.y / grid.cell_size));
                grid.resize(width, height);
            }
        }
    }

    if (state.placement_tool_active) {
        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 100), ImVec2(200, FLT_MAX));
        ImGui::Begin("Placement tool", &state.placement_tool_active, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Shape"))
            {
                if (ImGui::MenuItem("Single Particle", "Ctrl+P")) { state.editor_mode = single; }
                if (ImGui::MenuItem("Rectangle", "Ctrl+R"))   { state.editor_mode = rectangle; }
                if (ImGui::MenuItem("Sphere", "Ctrl+S"))   { state.editor_mode = sphere; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::ColorEdit4("Color", &state.selected_color[0]);
        ImGui::Checkbox("Boundary", &state.place_boundary);
        ImGui::SameLine();
        ImGui::Checkbox("Remove", &state.edit_delete);
        if (state.editor_mode == rectangle) {
            ImGui::InputInt2("n", state.rect_n);
            ImGui::SliderAngle("r", &state.rect_r);
        }

        if (state.editor_mode == sphere) {
            ImGui::InputFloat("radius", &state.sphere_radius);
        }

        if (ImGui::Button("Place") || (ImGui::IsKeyPressed(ImGuiKey_Enter) && !state.currently_typing)) {
            solver.add_particle_grid(preview_particles);
            solver.update_nu_boundaries(grid);
        }
        if (ImGui::Button("Clear")) {
            solver.particles.clear();
        }
        ImGui::End();

        preview_particles.clear(); // TODO: maybe we can only update when something actually changes?
        const glm::vec3 color{state.selected_color[0], state.selected_color[1], state.selected_color[2]};
        const float m_i = solver.get_particle_mass();

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
            state.placement_origin = camera.get_cursor_world_pos(window);
        }

        // TODO: no particle overlap, I need to use a grid or something for initialisation!
        // TODO: make it possible to avoid storing parameters

        if (state.editor_mode == rectangle) {
            const glm::mat4 R = glm::rotate(glm::mat4(1.0f), state.rect_r, glm::vec3(0,0,1));
            const float x_offset = 0.5f * state.rect_n[0] * solver.h;
            const float y_offset = 0.5f * state.rect_n[1] * solver.h;
            for (int x = 0; x < state.rect_n[0]; x++) {
                for (int y = 0; y < state.rect_n[1]; y++) {
                    const glm::vec4 r_pos = glm::vec4(x - x_offset, y - y_offset, 0.0f, 0.0f) * R;
                    const glm::vec2 pos = state.placement_origin + glm::vec2(r_pos.x, r_pos.y) * solver.h;
                    preview_particles.add(pos, {0,0}, {0,0}, m_i, solver.rho_0, state.place_boundary, color);
                }
            }
        } else if (state.editor_mode == sphere) {
            if (state.sphere_radius > 0.0f) {
                const int radius_steps = static_cast<int>(std::ceil(state.sphere_radius / solver.h));
                const float radius2 = state.sphere_radius * state.sphere_radius;

                for (int x = -radius_steps; x <= radius_steps; x++) {
                    for (int y = -radius_steps; y <= radius_steps; y++) {
                        const glm::vec2 offset = glm::vec2(x, y) * solver.h;
                        if (glm::dot(offset, offset) > radius2) continue;

                        const glm::vec2 pos = state.placement_origin + offset;
                        preview_particles.add(pos, {0,0}, {0,0}, m_i, solver.rho_0, state.place_boundary, color);
                    }
                }
            }
        }else {
            const glm::vec2 pos = state.placement_origin + glm::vec2(state.rect_n[0], state.rect_n[1]) * solver.h;
            preview_particles.add(pos, {0,0}, {0,0}, m_i, solver.rho_0, state.place_boundary, color);
        }
    }else {
        preview_particles.clear();
    }
}

void Application::ui_view_plot() {
    ImGui::Begin("Statistics viewer", nullptr);
    ImGui::End();
}
