#ifndef SPH_FLUID_SOLVER_APP_STATE_H
#define SPH_FLUID_SOLVER_APP_STATE_H
#include <glm/vec2.hpp>

#include "particle.h"

enum AppMode {
    simulate,
    edit_scene,
    view_plot
};

enum EditorMode {
    single,
    rectangle
};

struct AppState {
    bool paused = true;
    bool recording = false;
    bool spigot_enabled = false;
    bool currently_typing = false;
    bool camera_moving = false;

    char input_buffer0[32] = "";
    char input_buffer1[32] = "";
    char input_buffer2[32] = "";

    // visual
    int selected_particle_index = -1;
    bool draw_neighbors = false;

    // editor
    bool placement_tool_active = true;
    float selected_color[4]{1,0,0,1};
    bool place_boundary = false;
    EditorMode editor_mode = rectangle;
    bool edit_delete = false;
    int rect_n [2]{0, 0};
    float rect_r = 0.0f;
    glm::vec2 placement_origin{0, 0};

    // TODO: proper input handler
    bool space_last_pressed = false;
    bool tab_last_pressed = false;

    AppMode app_mode = simulate;

    float spigot_cooldown = 0.0f;
    float recording_frame_time = 0.0f;
    int recorded_frame_count = 0;

    double frame_time = 0.0;

    // statistics
    bool plot_enabled = false;
    bool move_with_plot = false;
    double x_min = -1.0, x_max = 60.0, y_min = 0.7, y_max = 1.4;
    int stat_seq_index = 0;
    float stat_seq_time = 0;
};

#endif //SPH_FLUID_SOLVER_APP_STATE_H
