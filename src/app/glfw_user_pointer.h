#ifndef SPH_FLUID_SOLVER_GLFW_USER_POINTER_H
#define SPH_FLUID_SOLVER_GLFW_USER_POINTER_H
#include "camera.h"

struct GLFWUserPointer {
    Camera2D &camera;
    AppState &state;
};

#endif //SPH_FLUID_SOLVER_GLFW_USER_POINTER_H
