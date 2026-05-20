#include "solver.h"
#include <execution>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <glm/ext/matrix_transform.hpp>
#include "sph_integrators.h"
#include "sph_kernel.h"

FluidSolver::FluidSolver(const float dt, const float h, const float rho_0, const float k, const float nu, const glm::vec2 g) {
    this->dt = dt;
    this->h = h;
    this->rho_0 = rho_0;
    this->k = k;
    this->nu = nu;
    this->g = g;
    this->max_v = 0;
}

void FluidSolver::add_particle(const glm::vec2 o, const glm::vec3 color, const bool is_fixed) {
    particles.emplace_back(Particle2D(o, glm::vec2(0), glm::vec2(0), get_particle_mass(), 0, rho_0, color, is_fixed));
}

void FluidSolver::add_particle_grid(const glm::ivec2 N, const glm::vec2 o, const glm::vec3 color, const bool is_fixed, const float r) {
    const float m_i = get_particle_mass();
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(r), glm::vec3(0,0,1));
    for (int y = 0; y < N.y; y++ ) {
        for (int x = 0; x < N.x; x++) {
            const glm::vec4 r_pos = glm::vec4(x, y, 0.0f, 0.0f) * R;
            const glm::vec2 pos = o + glm::vec2(r_pos.x, r_pos.y) * h;
            particles.emplace_back(Particle2D(pos, glm::vec2(0), glm::vec2(0), m_i, 0, rho_0, color, is_fixed));
        }
    }
}

void FluidSolver::add_particle_grid(const std::vector<Particle2D> &p_other) {
    for (auto &p : p_other) {
        this->particles.emplace_back(p);
    }
}

float FluidSolver::get_particle_mass() const {
    return h * h * rho_0;
}

float FluidSolver::density_explicit(const int i) const {
    float sum = 0;
    const Particle2D &p_i = particles[i];
    for (const auto j: neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        sum += p_j.mass * sph::kernels::cubic_spline_2D(p_i.pos, p_j.pos, h);
    }
    return sum;
}

float FluidSolver::pressure(const int i) const {
    const Particle2D &p_i = particles[i];
    return  glm::max(k * (p_i.density / rho_0 - 1), 0.0f);
}

glm::vec2 FluidSolver::pressure_acceleration(const int i) const {
    const Particle2D &p_i = particles[i];
    auto sum = glm::vec2(0);
    const float i_frac = p_i.pressure / (p_i.density * p_i.density);
    for (const auto j : neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        sum += p_j.mass * (i_frac + p_j.pressure / (p_j.density * p_j.density)) * sph::kernels::cubic_spline_2D_deriv(p_i.pos, p_j.pos, h);
    }
    return -sum;
}

glm::vec2 FluidSolver::viscosity_acceleration(const int i) const {
    const Particle2D &p_i = particles[i];
    auto sum = glm::vec2(0);
    for (const auto j : neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        glm::vec2 x_ij = p_j.pos - p_i.pos;
        sum += p_j.mass / p_j.density * ((p_j.vel - p_i.vel) * x_ij) / (x_ij * x_ij + 0.01f * h * h) * sph::kernels::cubic_spline_2D_deriv(p_i.pos, p_j.pos, h);
    }
    return 2 * nu * sum;
}

glm::vec2 FluidSolver::gravity_acceleration(int i) const {
    const Particle2D &p_i = particles[i];
    return g / p_i.mass;
}

void FluidSolver::update_neighbors() {
    neighbor_indices.clear();
    for (int i = 0; i < particles.size(); i++) {
        neighbor_indices.emplace_back();
        for (int j = 0; j < particles.size(); j++) {
            if (const float d = length(particles[j].pos - particles[i].pos); d <= 2 * h) {
                neighbor_indices[i].push_back(j);
            }
        }
    }
}

void FluidSolver::update_neighbors_parallel() {
    int N = get_num_particles();
    neighbor_indices = std::vector<std::vector<int>>(N);
    std::vector<int> indices(N);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](int i) {
        std::vector<int> local;
        const Particle2D &p_i = particles[i];
        for (int j = 0; j < particles.size(); j++) {
                const Particle2D &p_j = particles[j];
            glm::vec2 d = p_j.pos - p_i.pos;
            if (glm::dot(d, d) <= (2*h)*(2*h)) {
                    local.push_back(j);
            }
        }
        neighbor_indices[i] = std::move(local);
    });
}

void FluidSolver::step(DataStructure::Grid &grid) {
    // TODO: measure timing for different parts!
    max_v = 0.0f;
    grid.populate_cells();
    neighbor_indices = grid.calculate_neighbors(h);
    std::vector<int> temp_remove_indices;
    //update_neighbors();
    //update_neighbors_parallel();

    for (int i = 0; i < particles.size(); i++) {
        particles[i].density = density_explicit(i);
        particles[i].pressure = pressure(i);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.acc = glm::vec2(0);
        p_i.acc += gravity_acceleration(i);
        p_i.acc += viscosity_acceleration(i);
        p_i.acc += pressure_acceleration(i);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.vel = sph::integrators::euler_cromer_vel_step(p_i.vel, p_i.acc, dt);
        if (const float v_abs = glm::length(p_i.vel); v_abs > max_v) max_v = v_abs; // update for cfl
        p_i.pos = sph::integrators::euler_cromer_pos_step(p_i.pos, p_i.vel, dt);
        if (grid.get_cell_index(i).x == -1) {
            temp_remove_indices.push_back(i);
        }
    }

    for (const int i : temp_remove_indices) {
        particles[i] = particles.back();
        particles.pop_back();
    }
}

void FluidSolver::step(DataStructure::Grid &grid, const float step_dt) {
    dt = step_dt;
    step(grid);
}

void FluidSolver::clean_particles() {
    particles.clear();
    neighbor_indices.clear();
}

std::vector<Particle2D> FluidSolver::get_neighbors(const int i) {
    std::vector<Particle2D> neighbors{};
    for (const auto j : neighbor_indices[i]) {
        neighbors.emplace_back(particles[j]);
    }
    return neighbors;
}

float FluidSolver::get_cfl_lambda() const {
    return dt * max_v / h;
}
