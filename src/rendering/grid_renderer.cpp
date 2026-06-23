#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <string>
#include "shader_utilities.h"
#include "grid_renderer.h"

bool GridRenderer::init() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    shader_program = createShaderProgram(
        loadFile(std::string(SHADER_DIR) + "grid.vert"),
        loadFile(std::string(SHADER_DIR) + "grid.frag")
        );
    vao = createVAO();
    inv_transform_loc = glGetUniformLocation(shader_program, "uInvTransform");
    origin_loc = glGetUniformLocation(shader_program, "uOrigin");
    size_loc = glGetUniformLocation(shader_program, "uSize");
    return true;
}

void GridRenderer::render(const Grid &grid, const Camera2D &camera) const {
    glm::mat4 invCameraTransform = glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix() * glm::mat4(1.0f));

    glUseProgram(shader_program);
    glBindVertexArray(vao);

    glUniformMatrix4fv(inv_transform_loc, 1, GL_FALSE, &invCameraTransform[0][0]);
    glUniform2f(origin_loc, grid.origin.x, grid.origin.y);
    glUniform2f(size_loc, static_cast<float>(grid.width) * grid.cell_size, static_cast<float>(grid.height) * grid.cell_size);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
}
