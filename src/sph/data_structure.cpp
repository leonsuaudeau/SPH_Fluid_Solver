#include "data_structure.h"

DataStructure::Grid::Grid(const int width, const int height, const float h, const glm::vec2 origin) {
    const std::vector<Cell> _cells(width * height);
    this->cells = _cells;
    this-> cell_size = h;
    this -> width = width;
    this -> height = height;
    this -> origin = origin;
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
        cells[c_i].p_indices.push_back(i);
    }
}

std::vector<std::vector<int>> DataStructure::Grid::calculate_neighbors() {
    std::vector<std::vector<int>> neighbor_indices;
    for (int i = 0; i < particles.size(); i++) {
        neighbor_indices.emplace_back();
        for (int j = 0; j < particles.size(); j++) {
            if (const float d = length(particles[j].pos - particles[i].pos); d <= 2 * h) {
                neighbor_indices[i].push_back(j);
            }
        }
    }
    return neighbor_indices;
}
