#include "particle_renderer.h"
#include "shader_utilities.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <string>

bool ParticleRenderer::init() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    shader_program = createShaderProgram(
        loadFile(std::string(SHADER_DIR) + "particle.vert"),
        loadFile(std::string(SHADER_DIR) + "particle.frag")
        );
    vao = createVAO();
    transform_loc = glGetUniformLocation(shader_program, "uTransform");
    center_loc = glGetUniformLocation(shader_program, "uCenter");
    radius_loc = glGetUniformLocation(shader_program, "uRadius");
    color_loc = glGetUniformLocation(shader_program, "uColor");
    return true;
}

void ParticleRenderer::render(const FluidSolver &solver, const AppState &state, const Camera2D &camera, const int width, const int height) const {
    glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);
    glViewport(0, 0, width, height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    float radius = solver.h / 2.0f;

    for (const auto& p : solver.particles) {
        glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &cameraTransform[0][0]);
        glUniform2f(center_loc, p.pos.x, p.pos.y);
        glUniform1f(radius_loc, radius);
        // debug pressure coloring TODO: this should be done in the shader, not here
        glm::vec3 red{1,0,0};
        glm::vec3 blue{0,0,1};
        const float v = glm::length(p.vel);
        //glm::vec3 color = p.color;
        const glm::vec3 color = p.is_fixed? p.color : glm::mix(blue, red, log(v + 1e-4) / 5);
        glUniform3f(color_loc, color.r,color.g,color.b);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // TODO: !!very important, separate coloring logic this is just a hack
    if (state.draw_neighbors && state.selected_particle_index >= 0) {
        for (const auto& i: solver.neighbor_indices[state.selected_particle_index]) {
            glm::vec3 color{0, 0, 0};
            if (i == state.selected_particle_index) {
                color = glm::vec3{1,1,1};
            }
            const Particle2D &p = solver.particles[i];
            glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &cameraTransform[0][0]);
            glUniform2f(center_loc, p.pos.x, p.pos.y);
            glUniform1f(radius_loc, radius);
            glUniform3f(color_loc, color.r, color.g, color.b);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }

    glBindVertexArray(0);
}

void ParticleRenderer::render(const std::vector<Particle2D> &particles, const float radius, const Camera2D &camera, int width, int height) const {
    glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);

    glBindVertexArray(vao);
    for (const auto& p : particles) {
        glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &cameraTransform[0][0]);
        glUniform2f(center_loc, p.pos.x, p.pos.y);
        glUniform1f(radius_loc, radius);
        const glm::vec3 color = p.color;

        glUniform3f(color_loc, color.r,color.g,color.b);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
}
