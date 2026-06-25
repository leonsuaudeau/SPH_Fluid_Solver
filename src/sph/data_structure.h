#ifndef SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#define SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#include <vector>
#include "particle.h"

constexpr int MAX_PARTICLES_PER_CELL = 64;
constexpr int MAX_NEIGHBORS = 128;

class FluidSolver;

struct NeighborList {
    void add(int i, int j);
    void clear();
    std::vector<int> counts = std::vector<int>(MAX_PARTICLES);
    std::vector<int> neighbors = std::vector<int>(MAX_PARTICLES * MAX_NEIGHBORS);
};

struct Grid {
    Grid (int width, int height, glm::vec2 origin, float cell_size, Particles &particles);
    void populate_cells();
    void calculate_neighbors(float h, NeighborList &neighbors) const;
    void calculate_neighbors(int i, float h, NeighborList &neighbors, int &total_neighbor_overflow_count) const;
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
