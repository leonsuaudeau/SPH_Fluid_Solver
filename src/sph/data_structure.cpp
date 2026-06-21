#include "data_structure.h"
#include <execution>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include <thread>

#include "solver.h"

Grid::Grid(const int width, const int height, const glm::vec2 origin, const float cell_size, Particles &particles):
    particles(particles){
    this->width = width;
    this->height = height;
    this->cell_size = cell_size;
    this->inv_cell_size = 1.0f / cell_size;
    this->origin = origin;
    const int cell_count = width * height;
    counts = std::vector<int>(cell_count);
    particle_indices = std::vector<int>(cell_count * MAX_PARTICLES_PER_CELL);
}

int Grid::get_cell_index(const float p_x, const float p_y) const {
    const int i = static_cast<int>((p_x - origin.x) * inv_cell_size);
    const int j = static_cast<int>((p_y - origin.y) * inv_cell_size);
    if (!is_inside(i, j)) return -1;
    return i + j * width;
}

bool Grid::is_inside(const int x, const int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

void Grid::populate_cells() {
    for (int i = 0; i < width * height; i++) counts[i] = 0;

    for (int i = 0; i < particles.count; i++) {
        const int cell = get_cell_index(particles.p_x[i], particles.p_y[i]);
        if (cell == -1) continue;

        int &count = counts[cell];
        if (count < MAX_PARTICLES_PER_CELL) {
            particle_indices[cell * MAX_PARTICLES_PER_CELL + count] = i;
            count++;
        }else {
            std::cout << "Grid cell overflow!" << std::endl;
        }
    }
}

void Grid::calculate_neighbors(const float h, std::vector<std::vector<int>> &neighbor_indices) const {
    const float radius2 = 4.0f * h * h + 0.0001f;

    neighbor_indices.resize(particles.count);
    for (auto& n : neighbor_indices) {
        n.clear();
        if (n.capacity() < 16) {
            n.reserve(16);
        }
    }
    for (int i = 0; i < particles.count; i++) {
        const float p_x_i = particles.p_x[i];
        const float p_y_i = particles.p_y[i];

        const int c_x_i =  static_cast<int>(std::floor((p_x_i - origin.x) * inv_cell_size));
        const int c_y_i = static_cast<int>(std::floor((p_y_i - origin.y) * inv_cell_size));

        for (int o_y = -2; o_y <= 2; o_y++) {
            const int c_y_j = c_y_i + o_y;
            for (int o_x = -2; o_x <= 2; o_x++) {
                const int c_x_j = c_x_i + o_x;
                if (!is_inside(c_x_j, c_y_j)) continue;

                const int c_j = c_x_j + c_y_j * width;

                const int offset = c_j * MAX_PARTICLES_PER_CELL;
                const int count = counts[c_j];
                for (int k = 0; k < count; k++) {
                    int j = particle_indices[offset + k];
                    if (j == i) continue;
                    const float d_x = particles.p_x[j] - p_x_i;
                    const float d_y = particles.p_y[j] - p_y_i;
                    if (const float d2 = d_x*d_x + d_y*d_y; d2 < radius2) {
                        neighbor_indices[i].push_back(j);
                    }
                }
            }
        }
    }
}