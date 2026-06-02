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
        [[nodiscard]] glm::ivec2 get_cell_index(int i) const;
        [[nodiscard]] glm::ivec2 get_cell_index(glm::vec2 pos) const;
        [[nodiscard]] std::vector<std::vector<int>> calculate_neighbors(float h) const;
        bool is_inside(int x, int y) const;

        std::vector<Particle2D> &particles;
        std::vector<std::vector<Cell>> cells;
        float cell_size;
        int width, height;
        glm::vec2 origin{};
    };
}

#endif //SPH_FLUID_SOLVER_DATA_STRUCTURE_H
