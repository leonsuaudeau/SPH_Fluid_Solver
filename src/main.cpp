#include <cstdio>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <glm/vec2.hpp>
#include "shader_utilities.h"
#include "particle.h"
#include "camera.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    std::vector<Particle> particles;
    Camera2D camera(glm::vec2(0, 0), 20, 0.1f);

    for (int y = -8; y <= 8; y++) {
        for (int x = -10; x <= 10; x++) {
            particles.emplace_back(Particle(x, y, 0, 0, 0, 0, 1));
        }
    }
    std::cout << "Number of particles: " << particles.size() << std::endl;

    // Create GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

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
        loadFile("shaders/particle.vert"),
        loadFile("shaders/particle.frag")
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Test Window");
        ImGui::Text("Hello, world!");
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        camera.updateTransform(display_w, display_h);
        glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        for (auto& p : particles) {
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &cameraTransform[0][0]);
            glUniform2f(centerLoc, p.x, p.y);
            glUniform1f(radiusLoc, 0.1f);
            glUniform3f(colorLoc, 1,0,0);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
        glBindVertexArray(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}