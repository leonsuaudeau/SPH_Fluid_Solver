#ifndef SPH_FLUID_SOLVER_SCREEN_RECORDER_H
#define SPH_FLUID_SOLVER_SCREEN_RECORDER_H

class ScreenRecorder {
public:
    void update(float dt, bool enabled, int width, int height);
private:
    float accumulated_time = 0.0f;
    int frame_count = 0;
};

#endif //SPH_FLUID_SOLVER_SCREEN_RECORDER_H
