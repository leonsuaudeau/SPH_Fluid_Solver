#ifndef SPH_FLUID_SOLVER_APPLICATION_H
#define SPH_FLUID_SOLVER_APPLICATION_H
#include <string>

#include "app_state.h"
#include "camera.h"
#include "particle_renderer.h"
#include "solver.h"
#include "screen_recorder.h"
#include "ui_handling.h"

struct GLFWwindow;

class Application {
public:
    Application();
    int run();
    void ui_scene_editor();
    void ui_simulate();
private:
    GLFWwindow*window = nullptr;
    Camera2D camera;
    FluidSolver solver;
    AppState state;
    ParticleRenderer particle_renderer;
    ScreenRecorder screen_recorder;
    ImGuiLayer ui_handler;
    std::vector<std::string> scenes;
    std::vector<std::string> snapshots;
    std::vector<Particle2D> preview_particles;
};

#endif //SPH_FLUID_SOLVER_APPLICATION_H
