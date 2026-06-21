#ifndef SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#define SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#include <vector>
#include "particle.h"

constexpr int MAX_PARTICLES_PER_CELL = 64;

class FluidSolver;

struct Grid {
    Grid (int width, int height, glm::vec2 origin, float cell_size, Particles &particles);
    void populate_cells();
    void calculate_neighbors(float h, std::vector<std::vector<int>> &neighbor_indices) const;
    [[nodiscard]] int get_cell_index(float p_x, float p_y) const;
    [[nodiscard]] bool is_inside(int x, int y) const;

    int width, height;
    float cell_size;
    float inv_cell_size;
    glm::vec2 origin{};
    std::vector<int> counts;
    std::vector<int> particle_indices;
    Particles &particles;
};

#endif //SPH_FLUID_SOLVER_DATA_STRUCTURE_H
