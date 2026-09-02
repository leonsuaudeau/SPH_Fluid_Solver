#include "data_structure.h"
#include <algorithm>
#include <cmath>
#include <execution>
#include <iostream>
#include <thread>
#include "solver.h"

Grid::Grid(const int width, const int height, const glm::vec2 origin, const float cell_size, Particles &particles):
    particles(particles){
    this->origin = origin;
    set_cell_size(cell_size);
    resize(width, height);
}

void Grid::resize(const int new_width, const int new_height) {
    width = std::max(new_width, 0);
    height = std::max(new_height, 0);

    const auto cell_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    counts.assign(cell_count, 0);
    particle_indices.assign(cell_count * MAX_PARTICLES_PER_CELL, -1);
}

void Grid::set_cell_size(const float new_cell_size) {
    if (new_cell_size <= 0.0f) return;
    cell_size = new_cell_size;
    inv_cell_size = 1.0f / cell_size;
}

int Grid::get_cell_index(const float p_x, const float p_y) const {
    const int i = static_cast<int>(std::floor((p_x - origin.x) * inv_cell_size));
    const int j = static_cast<int>(std::floor((p_y - origin.y) * inv_cell_size));
    if (!is_inside(i, j)) return -1;
    return i + j * width;
}

bool Grid::is_inside(const int x, const int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

void Grid::populate_cells() {
    std::fill(counts.begin(), counts.end(), 0);

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

void Grid::calculate_neighbors(const int i, const float h, NeighborList &neighbors, int &max_neighbor_overflow_count) const {
    const float radius2 = 4.0f * h * h + 0.0001f;

    const float p_x_i = particles.p_x[i];
    const float p_y_i = particles.p_y[i];

    const int c_x_i =  static_cast<int>(std::floor((p_x_i - origin.x) * inv_cell_size));
    const int c_y_i = static_cast<int>(std::floor((p_y_i - origin.y) * inv_cell_size));

    int neighbor_count = 0;
    int neighbor_overflow_count = 0;
    const int neighbor_offset = i * MAX_NEIGHBORS;

    for (int o_y = -1; o_y <= 1; o_y++) {
        const int c_y_j = c_y_i + o_y;
        for (int o_x = -1; o_x <= 1; o_x++) {
            const int c_x_j = c_x_i + o_x;
            if (!is_inside(c_x_j, c_y_j)) continue;

            const int c_j = c_x_j + c_y_j * width;

            const int cell_offset = c_j * MAX_PARTICLES_PER_CELL;
            const int count = counts[c_j];
            for (int k = 0; k < count; k++) {
                int j = particle_indices[cell_offset + k];
                if (j == i) continue;
                const float d_x = particles.p_x[j] - p_x_i;
                const float d_y = particles.p_y[j] - p_y_i;
                const float d2 = d_x*d_x + d_y*d_y;
                if (d2 < radius2) {
                    if (neighbor_count < MAX_NEIGHBORS) {
                        neighbors.neighbors[neighbor_offset + neighbor_count] = j;
                        neighbor_count++;
                    }else {
                        neighbor_overflow_count++;
                    }
                }
            }
        }
    }
    neighbors.counts[i] = neighbor_count;
    max_neighbor_overflow_count = neighbor_overflow_count > max_neighbor_overflow_count ? neighbor_overflow_count : max_neighbor_overflow_count;
}

void NeighborList::clear() {
    for (int i = 0; i < MAX_PARTICLES; i++) counts[i] = 0;
}
