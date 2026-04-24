#include "camera.h"
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

void Camera2D::zoom(const float amount) {
    scale += amount;
}

void Camera2D::updateTransform(const int screenWidth, const int screenHeight) {
    const float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    const float viewHeight = scale;
    const float viewWidth = viewHeight * aspectRatio;
    projection = glm::ortho(position.x - viewWidth * 0.5f, position.x + viewWidth * 0.5f, position.y - viewHeight * 0.5f, position.y + viewHeight * 0.5f, -1.0f, 1.0f);
    view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}
