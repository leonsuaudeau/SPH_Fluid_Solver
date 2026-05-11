#ifndef SPH_FLUID_SOLVER_UI_HANDLING_H
#define SPH_FLUID_SOLVER_UI_HANDLING_H
#include "app_state.h"
#include "camera.h"
#include "solver.h"

struct GLFWwindow;

class ImGuiLayer {
public:
    static void init(GLFWwindow*  window, float scale);
    static void beginFrame();
    static void render();
    static void shutdown();
};

namespace Ui {
    void set_debug_overlay(const AppState &state, const FluidSolver &solver, const Camera2D &camera);
}

#endif //SPH_FLUID_SOLVER_UI_HANDLING_H
