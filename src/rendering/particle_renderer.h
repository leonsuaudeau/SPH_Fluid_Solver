#ifndef SPH_FLUID_SOLVER_PARTICLE_RENDERER_H
#define SPH_FLUID_SOLVER_PARTICLE_RENDERER_H
#include <glad/glad.h>
#include "camera.h"
#include "solver.h"

class ParticleRenderer {
public:
    bool init();
    void render(const FluidSolver &solver, const Camera2D &camera, int width, int height) const;
private:
    GLuint shader_program = 0;
    GLuint vao = 0;
    GLint transform_loc = -1;
    GLint center_loc = -1;
    GLint radius_loc = -1;
    GLint color_loc = -1;
};

#endif //SPH_FLUID_SOLVER_PARTICLE_RENDERER_H
