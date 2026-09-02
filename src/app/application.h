#ifndef SPH_FLUID_SOLVER_APPLICATION_H
#define SPH_FLUID_SOLVER_APPLICATION_H
#include <string>
#include "app_state.h"
#include "camera.h"
#include "grid_renderer.h"
#include "object.h"
#include "particle_renderer.h"
#include "solver.h"
#include "screen_recorder.h"
#include "statistics.h"
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
    Grid grid;
    AppState state;
    ParticleRenderer particle_renderer;
    GridRenderer grid_renderer;
    ScreenRecorder screen_recorder;
    ImGuiLayer ui_handler;
    std::vector<std::string> scenes;
    std::vector<std::string> snapshots;
    Particles preview_particles;
    stats::Sequence stat_seq{};
    std::vector<Object> objects;
};

#endif //SPH_FLUID_SOLVER_APPLICATION_H
