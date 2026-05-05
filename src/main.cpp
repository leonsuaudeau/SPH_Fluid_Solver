#include <cstdio>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <glm/vec2.hpp>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "camera.h"
#include "shader_utilities.h"
#include "solver.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void ShowDebugOverlay(float frame_time, float dt, float h, float cfl_lambda, float rho_0, float k, float nu, int particle_count, const Camera2D &camera) {
    ImGuiIO& io = ImGui::GetIO();

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
        ImGui::Text("dt: %.4f", dt);
        ImGui::Text("h: %.2f", h);
        ImGui::Text("CFL: %.2f", cfl_lambda);
        ImGui::Text("rho_0: %.2f", rho_0);
        ImGui::Text("k: %.2f", k);
        ImGui::Text("nu: %.2f", nu);
        ImGui::Text("Particles: %d", particle_count);
        ImGui::Separator();
        ImGui::Text("Delta time: %.3f ms (%.1f FPS)", frame_time * 1000.0f, 1.0f / frame_time);
        ImGui::Text("Camera:");
        glm::vec2 cam_pos = camera.getPosition();
        ImGui::Text("x: %.2f y: %.2f", cam_pos.x, cam_pos.y);
    }
    ImGui::End();
}

int main() {
    bool paused = true;
    bool record = false;
    bool spigot = false;
    bool click_spawn = false;
    float spigot_cooldown = 0;
    float recording_frame_time = 0;
    int frame_count = 0;
    std::filesystem::create_directories("../output");
    Camera2D camera(glm::vec2(0, 0), 80, 0.1f);
    FluidSolver solver(0.9f, 1.1f, 20000, 0.2f, glm::vec2(0, -9.81f));

    constexpr float cfl_lambda = 0.4f;
    const float dt = solver.get_cfl_timestep(cfl_lambda, 120.0f);
    std::cout << "CLF number: " << dt << std::endl;

    // Create GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow(
        (int)(1280 * main_scale),
        (int)(800 * main_scale), "Fluid Solver", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return 1;
    }

    const GLuint shaderProgram = createShaderProgram(
        loadFile(std::string(SHADER_DIR) + "particle.vert"),
        loadFile(std::string(SHADER_DIR) + "particle.frag")
        );
    const GLuint vao = createVAO();
    const GLint transformLoc = glGetUniformLocation(shaderProgram, "uTransform");
    const GLint centerLoc = glGetUniformLocation(shaderProgram, "uCenter");
    const GLint radiusLoc = glGetUniformLocation(shaderProgram, "uRadius");
    const GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    double current_time = glfwGetTime();
    double frame_time = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        // TODO: replace by mouse controls, mouse3 and dragging for move, scroll for zoom
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.move(0, 1);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.move(0, -1);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.move(-1, 0);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.move(1, 0);

        if (!paused) {
            solver.step(dt);
            spigot_cooldown += dt;
            if (spigot && spigot_cooldown > 0.05f) {
                spigot_cooldown = 0;
                solver.add_particle_grid({3,1}, {-48,60}, {1,0,0});
                solver.particles[solver.particles.size() - 1].vel = glm::vec2(0, -20);
                solver.particles[solver.particles.size() - 2].vel = glm::vec2(0, -20);
                solver.particles[solver.particles.size() - 3].vel = glm::vec2(0, -20);
            }
            //std::cout << solver.get_particle_density(0) << std::endl;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

        if (click_spawn && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
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
        if (ImGui::Button(paused ? "Resume" : "Pause")) {
            paused = !paused;
        }
        if (ImGui::Button("Scene 1")) {
            solver.clean_particles();
            solver.add_particle({0,10}, {1,0,0});
            solver.add_particle_grid({40, 3}, {-20, -10}, {0.25,0.25,0.25}, true);
        }
        if (ImGui::Button("Scene 2")) {
            solver.clean_particles();
            solver.add_particle_grid({100, 30}, {-50, -7.3}, {0,0,1});
            solver.add_particle_grid({100, 3}, {-50, -10}, {0.25,0.25,0.25}, true);
            solver.add_particle_grid({3, 80}, {-52.7, -10}, {0.25,0.25,0.25}, true);
            solver.add_particle_grid({3, 80}, {40, -10}, {0.25,0.25,0.25}, true);
        }
        if (ImGui::Button("Scene 3")) {
            solver.clean_particles();
            solver.add_particle_grid({40, 50}, {-50, -7.3}, {0,0,1});
            solver.add_particle_grid({100, 3}, {-50, -10}, {0.25,0.25,0.25}, true);
            solver.add_particle_grid({3, 80}, {-52.7, -10}, {0.25,0.25,0.25}, true);
            solver.add_particle_grid({3, 70}, {-14, -5}, {0.25,0.25,0.25}, true);
            solver.add_particle_grid({3, 80}, {40, -10}, {0.25,0.25,0.25}, true);
        }
        if (ImGui::Button("Add cubes")) {
            solver.add_particle_grid({10, 10}, {-5, 40}, {1,0,0});
            solver.add_particle_grid({10, 10}, {-5, 60}, {0,1,0});
            solver.add_particle_grid({10, 10}, {-5, 80}, {1,1,0});
        }
        if (ImGui::Button(spigot? "Turn off spigot" : "Turn on spigot")) {
            spigot = !spigot;
        }
        if (ImGui::Button(click_spawn? "Mouse spawn particle off" : "Mouse spawn particle on")) {
            click_spawn = !click_spawn;
        }
        ImGui::End();

        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Camera control", nullptr, flags);
        if (ImGui::Button("Reset")) {
            camera.setPosition(0, 0);
        }
        if (ImGui::Button(record ? "Pause recording" : "Resume recording")) {
            record = !record;
        }
        ImGui::End();
        ShowDebugOverlay(frame_time, dt, solver.h, cfl_lambda, solver.rho_0, solver.k, solver.nu, solver.get_num_particles(), camera);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        camera.updateTransform(display_w, display_h);
        glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        float radius = solver.h / 2.0f;
        for (const auto& p : solver.particles) {
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &cameraTransform[0][0]);
            glUniform2f(centerLoc, p.pos.x, p.pos.y);
            glUniform1f(radiusLoc, radius);
            // debug pressure coloring TODO: this should be done in the shader, not here
            glm::vec3 red{1,0,0};
            glm::vec3 blue{0,0,1};
            float v = glm::length(p.vel);
            //glm::vec3 color = p.color;
            glm::vec3 color = (p.is_fixed)? p.color : glm::mix(blue, red, log(v + 1e-12) / 5);

            glUniform3f(colorLoc, color.r,color.g,color.b);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
        glBindVertexArray(0);

        // Writing to file if needed
        recording_frame_time += dt;
        if (recording_frame_time > 0.0167f && record) {
            recording_frame_time = 0;
            std::vector<unsigned char> pixels(display_w * display_h * 3);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, display_w, display_h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
            std::ofstream out("../output/frame_" + std::to_string(frame_count++) + ".ppm", std::ios::binary);
            out << "P6\n" << display_w << " " << display_h << "\n255\n";

            for (int y = display_h - 1; y >= 0; --y) {
                out.write(reinterpret_cast<char*>(pixels.data() + y * display_w * 3), display_w * 3);
            }

            out.close();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        frame_time = glfwGetTime() - current_time;
        current_time = glfwGetTime();
    }
    /* // TODO: figure out reason for segfault (exit code 139)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    */
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}