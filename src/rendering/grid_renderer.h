#ifndef SPH_FLUID_SOLVER_GRID_RENDERER_H
#define SPH_FLUID_SOLVER_GRID_RENDERER_H
#include <glad/glad.h>
#include "camera.h"
#include "data_structure.h"

class GridRenderer {
public:
    bool init();
    void render(const Grid &grid, const Camera2D &camera) const;
private:
    GLuint shader_program = 0;
    GLuint vao = 0;
    GLint inv_transform_loc = -1;
    GLint origin_loc = -1;
    GLint size_loc = -1;
};

#endif //SPH_FLUID_SOLVER_GRID_RENDERER_H
