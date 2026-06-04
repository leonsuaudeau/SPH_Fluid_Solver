#include "solver.h"
#include <execution>
#include <algorithm>
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

glm::vec2 FluidSolver::combined_acceleration(const int i) const {
    const Particle2D &p_i = particles[i];
    auto pressure_sum = glm::vec2(0);
    auto viscosity_sum = glm::vec2(0);

    const glm::vec2 pos_i = p_i.pos;
    const glm::vec2 vel_i = p_i.vel;
    const float density_i = p_i.density;
    const float inv_density_i = 1.0f / density_i;
    const float i_frac = p_i.pressure * inv_density_i * inv_density_i;
    const float eps = 0.01f * h * h;

    for (const auto j : neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        const glm::vec2 kernel_deriv = sph::kernels::cubic_spline_2D_deriv(pos_i, p_j.pos, h);
        glm::vec2 x_ij = p_j.pos - pos_i;
        glm::vec2 v_ij = p_j.vel - vel_i;

        pressure_sum += p_j.mass * (i_frac + p_j.pressure / (p_j.density * p_j.density)) * kernel_deriv;
        viscosity_sum += p_j.mass / p_j.density * dot(v_ij, x_ij) / (dot(x_ij, x_ij) + eps) * kernel_deriv;
    }

    return -pressure_sum + 2.0f * nu * viscosity_sum;
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
            if (glm::vec2 d = particles[j].pos - particles[i].pos; glm::dot(d, d) <= 4.0f*h*h) {
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
                if (glm::vec2 d = p_j.pos - p_i.pos; glm::dot(d, d) <= 4.0f*h*h) {
                    local.push_back(j);
            }
        }
        neighbor_indices[i] = std::move(local);
    });
}

void FluidSolver::step(DataStructure::Grid &grid) {
    float max_v2 = 0.0f;

    grid.populate_cells();
    grid.calculate_neighbors(h, neighbor_indices);
    std::vector<int> temp_remove_indices;
    //update_neighbors();
    //update_neighbors_parallel();

    for (int i = 0; i < particles.size(); i++) {
        particles[i].density = density_explicit(i);
        particles[i].pressure = pressure(i);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        //if (p_i.is_fixed) continue;
        p_i.acc = glm::vec2(0);
        p_i.acc += gravity_acceleration(i);
        p_i.acc += combined_acceleration(i);
        //p_i.acc += viscosity_acceleration(i);
        //p_i.acc += pressure_acceleration(i);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.vel = sph::integrators::euler_cromer_vel_step(p_i.vel, p_i.acc, dt);

        if (const float v2 = glm::dot(p_i.vel, p_i.vel); v2 > max_v2) max_v2 = v2; // update for cfl

        p_i.pos = sph::integrators::euler_cromer_pos_step(p_i.pos, p_i.vel, dt);
        if (grid.get_cell_index(i).x == -1) {
            temp_remove_indices.push_back(i);
        }
    }

    max_v = std::sqrt(max_v2);

    for (const int i : temp_remove_indices) {
        particles[i] = particles.back();
        particles.pop_back();
    }
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
