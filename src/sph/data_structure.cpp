#include "data_structure.h"
#include "solver.h"

DataStructure::Grid::Grid(const int width, const int height, const glm::vec2 origin, FluidSolver &solver):
    particles(solver.particles),
    neighbor_offsets{
        -2 * width -2, -2 * width -1, -2 * width, -2 * width + 1, -2 * width + 2,
        -width -2, -width -1, -width, -width + 1, -width + 2,
        -2, -1, 0, 1, 2,
        width -2, width -1, width, width + 1, width + 2,
        2 * width -2, 2 * width -1, 2 * width, 2 * width + 1, 2 * width + 2
    } {
    const std::vector<Cell> _cells(width * height);
    this->max_cell = width * height - 1;
    this->cells = _cells;
    this->cell_size = solver.h;
    this->width = width;
    this->height = height;
    this->origin = origin;
}

int DataStructure::Grid::get_cell_index(const int i) const {
    const glm::vec2 diff = particles[i].pos - origin;
    const int x = diff.x / cell_size;
    const int y = diff.y / cell_size;
    return y * width + x;
}

void DataStructure::Grid::populate_cells() {
    for (auto &[p_indices] : cells) {
        p_indices.clear();
    }
    for (int i = 0; i < particles.size(); i++) {
        const int c_i = get_cell_index(i);
        if (c_i < 0 || c_i > max_cell) continue;
        cells[c_i].p_indices.push_back(i);
    }
}

bool DataStructure::Grid::is_inside_grid(const int i) const {
    const int c_i = get_cell_index(i);
    if (c_i < 0 || c_i > max_cell) {
        return false;
    }
    return true;
}

std::vector<std::vector<int>> DataStructure::Grid::calculate_neighbors(const float h) const {
    std::vector<std::vector<int>> neighbor_indices;
    for (int i = 0; i < particles.size(); i++) {
        neighbor_indices.emplace_back();
        const int c_i = get_cell_index(i);

        for (const int o : neighbor_offsets) {
            const int c_j = c_i + o;
            if (c_j < 0 || c_j > max_cell) continue;
            for (const int j : cells[c_j].p_indices) {
                if (const float d = length(particles[j].pos - particles[i].pos); d <= 2 * h) {
                    neighbor_indices[i].push_back(j);
                }
            }
        }
    }
    return neighbor_indices;
}
