#ifndef SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#define SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#include <vector>
#include "particle.h"

class FluidSolver;

namespace DataStructure {

    struct Cell {
        std::vector<int> p_indices;
    };

    struct Grid {
        Grid (int width, int height, glm::vec2 origin, FluidSolver &solver);
        void populate_cells();
        [[nodiscard]] int get_cell_index(int i) const;
        [[nodiscard]] std::vector<std::vector<int>> calculate_neighbors(float h) const;
        [[nodiscard]] bool is_inside_grid(int i) const;

        std::vector<Particle2D> &particles;
        std::vector<Cell> cells;
        int neighbor_offsets[25];
        int max_cell;
        float cell_size;
        int width, height;
        glm::vec2 origin{};
    };
}

#endif //SPH_FLUID_SOLVER_DATA_STRUCTURE_H
