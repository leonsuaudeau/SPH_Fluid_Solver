#ifndef SPH_FLUID_SOLVER_CAMERA_H
#define SPH_FLUID_SOLVER_CAMERA_H
#include "glm/glm.hpp"

class Camera2D {
public:
    Camera2D(glm::vec2 position, float scale, float speed);
    void move(float x, float y);
    void setPosition(float x, float y);
    void zoom(float amount);
    void updateTransform(int screenWidth, int screenHeight);
    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix () const;
    [[nodiscard]] glm::vec2 getPosition() const;
private:
    glm::mat4 projection{};
    glm::mat4 view{};
    glm::vec2 position{};
    float scale{};
    float speed{};
};

#endif //SPH_FLUID_SOLVER_CAMERA_H
