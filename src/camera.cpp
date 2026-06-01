#include "camera.h"

#include <GLFW/glfw3.h>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"

Camera2D::Camera2D(const glm::vec2 position, const float scale, const float speed) {
    move(position.x, position.y);
    zoom(scale);
    this->speed = speed;
}

glm::mat4 Camera2D::getViewMatrix() const {
    return view;
}

glm::mat4 Camera2D::getProjectionMatrix() const {
    return projection;
}

void Camera2D::move(const float x, const float y) {
    position.x += x * speed;
    position.y += y * speed;
}

void Camera2D::setPosition(const float x, const float y) {
    position = glm::vec2(x, y);
}

void Camera2D::zoom(const float amount) {
    scale += amount;
    scale = glm::max(scale, 0.0f);
}

void Camera2D::updateTransform(const int screenWidth, const int screenHeight) {
    const float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    const float viewHeight = scale;
    const float viewWidth = viewHeight * aspectRatio;
    projection = glm::ortho(-viewWidth * 0.5f, viewWidth * 0.5f, -viewHeight * 0.5f, viewHeight * 0.5f, -1.0f, 1.0f);
    view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}

glm::vec2 Camera2D::getPosition() const {
    return position;
}

float Camera2D::getScale() const {
    return scale;
}

glm::vec2 Camera2D::get_cursor_world_pos(GLFWwindow *window) const {
    double x, y;
    int display_w, display_h;
    glfwGetCursorPos(window, &x, &y);
    glfwGetFramebufferSize(window, &display_w, &display_h);
    const float ndcX = (2.0f * x) / display_w - 1.0f;
    const float ndcY = 1.0f - (2.0f * y) / display_h;
    const glm::mat4 inv_vp = glm::inverse(getProjectionMatrix() * getViewMatrix());
    glm::vec4 world_pos = inv_vp * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
    world_pos /= world_pos.w;
    return {world_pos.x, world_pos.y};
}
