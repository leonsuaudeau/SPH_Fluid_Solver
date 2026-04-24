#ifndef SPH_FLUID_SOLVER_MOTION_UPDATE_H
#define SPH_FLUID_SOLVER_MOTION_UPDATE_H
#include <glm/vec3.hpp>

glm::vec3 inline eulerCromerPositionStep(const glm::vec3 xT, const glm::vec3 vTNext, const float dt) {
    return xT + vTNext * dt;
}
glm::vec3 inline eulerCromerVelocityStep(const glm::vec3 vT, const glm::vec3 aT, const float dt) {
    return vT * aT *  dt;
}

#endif //SPH_FLUID_SOLVER_MOTION_UPDATE_H
