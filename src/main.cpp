#include <cstdio>
#include <iostream>
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

int main() {
    bool paused = true;
    Camera2D camera(glm::vec2(0, 0), 40, 0.1f);
    constexpr float dt = 0.006f;
    FluidSolver solver(0.9f, 1.1f, 2000, 0.0f, glm::vec2(0, -9.81f));

    solver.add_particle({0,0}, {1,0,0});
    solver.add_particle_grid({40, 3}, {-20, -10}, {0,1,0}, true);

    std::cout << "Number of particles: " << solver.get_num_particles() << std::endl;

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
    glfwSwapInterval(1);

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
            std::cout << solver.get_particle_density(0) << std::endl;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::Begin("Simulation control", nullptr, flags);
        if (ImGui::Button(paused ? "Resume" : "Pause")) {
            paused = !paused;
        }
        if (ImGui::Button("Reset")) {
            solver.clean_particles();
            solver.add_particle({0,0}, {1,0,0});
            solver.add_particle_grid({40, 3}, {-20, -10}, {0,1,0}, true);
        }
        ImGui::End();

        ImGui::SetNextWindowSizeConstraints( ImVec2(200, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Camera control", nullptr, flags);
        if (ImGui::Button("Reset")) {
            camera.setPosition(0, 0);
        }
        ImGui::End();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        camera.updateTransform(display_w, display_h);
        glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        float radius = solver.get_h() / 2.0f;
        for (const auto& p : solver.get_particles()) {
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &cameraTransform[0][0]);
            glUniform2f(centerLoc, p.pos.x, p.pos.y);
            glUniform1f(radiusLoc, radius);
            glUniform3f(colorLoc, p.color.r,p.color.g,p.color.b);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
        glBindVertexArray(0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
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