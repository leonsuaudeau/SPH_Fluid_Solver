#include "data_structure.h"
#include "solver.h"

DataStructure::Grid::Grid(const int width, const int height, const glm::vec2 origin, FluidSolver &solver):
    particles(solver.particles){
    const std::vector _cells(height, std::vector<Cell>(width));
    this->cells = _cells;
    this->cell_size = solver.h;
    this->width = width;
    this->height = height;
    this->origin = origin;
}

glm::ivec2 DataStructure::Grid::get_cell_index(const int i) const {
    const glm::vec2 diff = particles[i].pos - origin;
    const int x = diff.x / cell_size;
    const int y = diff.y / cell_size;
    if (!is_inside(x, y)) return {-1, -1};
    return {x, y};
}

bool DataStructure::Grid::is_inside(const int x, const int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

void DataStructure::Grid::populate_cells() {
    for (auto &row: cells) {
        for (auto &cell : row) {
            cell.p_indices.clear();
        }
    }

    for (int i = 0; i < particles.size(); i++) {
        const glm::ivec2 c_i = get_cell_index(i);
        if (c_i.x == -1) continue;
        cells[c_i.x][c_i.y].p_indices.push_back(i);
    }
}

std::vector<std::vector<int>> DataStructure::Grid::calculate_neighbors(const float h) const {
    std::vector<std::vector<int>> neighbor_indices;
    for (int i = 0; i < particles.size(); i++) {
        neighbor_indices.emplace_back();
        const glm::ivec2 c_i = get_cell_index(i);

        for (int o_y = -2; o_y <= 2; o_y++) {
            for (int o_x = -2; o_x <= 2; o_x++) {
                const glm::ivec2 c_j = c_i + glm::ivec2(o_x, o_y);
                if (!is_inside(c_j.x, c_j.y)) continue;
                for (const int j : cells[c_j.x][c_j.y].p_indices) {
                    if (const float d = length(particles[j].pos - particles[i].pos); d <= 2 * h) {
                        neighbor_indices[i].push_back(j);
                    }
                }
            }
        }
    }
    return neighbor_indices;
}
