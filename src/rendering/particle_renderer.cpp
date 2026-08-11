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

void ParticleRenderer::render(const FluidSolver &solver, const AppState &state, const Camera2D &camera, const int mode) const {
    glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    float radius = solver.h / 2.0f;

    for (int i = 0; i < solver.particles.count; i++) {
        glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &cameraTransform[0][0]);
        glUniform2f(center_loc, solver.particles.p_x[i], solver.particles.p_y[i]);
        glUniform1f(radius_loc, radius);
        // debug pressure coloring TODO: this should be done in the shader, not here
        glm::vec3 red{1,0,0};
        glm::vec3 blue{0,0,1};
        const float v = glm::length(glm::vec2(solver.particles.v_x[i], solver.particles.v_y[i]));

        const glm::vec3 color = (solver.particles.is_bound[i] || mode == 0)?
            glm::vec3(solver.particles.col_r[i], solver.particles.col_g[i], solver.particles.col_b[i])
            : glm::mix(blue, red, log(v + 1e-4) / 5);
        glUniform3f(color_loc, color.r,color.g,color.b);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // TODO: !!very important, separate coloring logic this is just a hack
    if (state.draw_neighbors && state.selected_particle_index >= 0) {

        const int start = state.selected_particle_index * MAX_NEIGHBORS;
        for (int curr = 0; curr < solver.neighbors.counts[state.selected_particle_index]; curr++) {
            const int j = solver.neighbors.neighbors[start + curr];

            glm::vec3 color{0, 0, 0};
            if (j == state.selected_particle_index) {
                color = glm::vec3{1,1,1};
            }
            glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &cameraTransform[0][0]);
            glUniform2f(center_loc, solver.particles.p_x[j], solver.particles.p_y[j]);
            glUniform1f(radius_loc, radius);
            glUniform3f(color_loc, color.r, color.g, color.b);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }

    glBindVertexArray(0);
}

void ParticleRenderer::render(const Particles &particles, const float radius, const Camera2D &camera) const {
    glm::mat4 cameraTransform = camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f);

    glBindVertexArray(vao);
    for (int i = 0; i < particles.count; i++) {
        glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &cameraTransform[0][0]);
        glUniform2f(center_loc, particles.p_x[i], particles.p_y[i]);
        glUniform1f(radius_loc, radius);
        const glm::vec3 color = glm::vec3(particles.col_r[i], particles.col_g[i], particles.col_b[i]);

        glUniform3f(color_loc, color.r,color.g,color.b);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
}
