#ifndef SPH_FLUID_SOLVER_GLFW_USER_POINTER_H
#define SPH_FLUID_SOLVER_GLFW_USER_POINTER_H

class Camera2D;
struct AppState;

struct GLFWUserPointer {
    Camera2D &camera;
    AppState &state;
};

#endif //SPH_FLUID_SOLVER_GLFW_USER_POINTER_H
