#ifndef SPH_FLUID_SOLVER_APP_STATE_H
#define SPH_FLUID_SOLVER_APP_STATE_H

struct AppState {
    bool paused = true;
    bool recording = false;
    bool spigot_enabled = false;
    bool click_spawn_enabled = false;
    bool currently_typing = false;
    bool camera_moving = false;

    float spigot_cooldown = 0.0f;
    float recording_frame_time = 0.0f;
    int recorded_frame_count = 0;

    double frame_time = 0.0;
};

#endif //SPH_FLUID_SOLVER_APP_STATE_H
