#ifndef SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#define SPH_FLUID_SOLVER_DATA_STRUCTURE_H
#include <vector>
#include "particle.h"

namespace DataStructure {

    struct Cell {
        std::vector<int> p_indices;
    };

    struct Grid {
        Grid (int width, int height, float h, glm::vec2 origin);
        void populate_cells();
        [[nodiscard]] int get_cell_index(int i) const;
        std::vector<std::vector<int>> calculate_neighbors();
        [[nodiscard]] int particle_count() const { return particles.size(); }

        std::vector<Particle2D> particles;
        std::vector<Cell> cells;
        float cell_size;
        int width, height;
        glm::vec2 origin;
    };
}

#endif //SPH_FLUID_SOLVER_DATA_STRUCTURE_H
