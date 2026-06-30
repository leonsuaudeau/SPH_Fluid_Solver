#include "solver.h"
#include <execution>
#include <chrono>
#include <cmath>
#include <iostream>
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
    particles.add(o, {0,0}, {0,0}, get_particle_mass(), rho_0, is_fixed, color);
}

void FluidSolver::add_particle_grid(const glm::ivec2 N, const glm::vec2 o, const glm::vec3 color, const bool is_fixed, const float r) {
    const float m_i = get_particle_mass();
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(r), glm::vec3(0,0,1));
    for (int y = 0; y < N.y; y++ ) {
        for (int x = 0; x < N.x; x++) {
            const glm::vec4 r_pos = glm::vec4(x, y, 0.0f, 0.0f) * R;
            const glm::vec2 pos = o + glm::vec2(r_pos.x, r_pos.y) * h;
            particles.add(pos, {0,0}, {0,0}, m_i, rho_0, is_fixed, color);
        }
    }
}

void FluidSolver::add_particle_grid(const Particles &p_other) {
    particles.combine(p_other);
}

float FluidSolver::get_particle_mass() const {
    return h * h * rho_0;
}

float FluidSolver::density_explicit(const int i, const sph::kernels::kernel_constants kernel_const) const {
    float sum = particles.m[i] * 4.0f * kernel_const.alpha; // density also needs to include the particle itself
    const float p_x_i = particles.p_x[i];
    const float p_y_i = particles.p_y[i];

    const int start = i * MAX_NEIGHBORS;
    for (int curr = 0; curr < neighbors.counts[i]; curr++) {
        const int j = neighbors.neighbors[start + curr];

        const float d_p_x = p_x_i - particles.p_x[j];
        const float d_p_y = p_y_i - particles.p_y[j];
        const float dot_p_p = d_p_x*d_p_x + d_p_y*d_p_y;
        sum += particles.m[j] * sph::kernels::cubic_spline_2D(dot_p_p, kernel_const);
    }
    return sum;
}

float FluidSolver::pressure(const int i) const {
    return glm::max(k * (particles.rho[i] / rho_0 - 1), 0.0f);
}

glm::vec2 FluidSolver::combined_acceleration(const int i, const sph::kernels::kernel_constants kernel_const,
    const std::vector<float> &p_over_rho2, const std::vector<float> &m_over_rho) const {
    float p_sum_x = 0;
    float p_sum_y = 0;
    float v_sum_x = 0;
    float v_sum_y = 0;

    const float p_x_i = particles.p_x[i];
    const float p_y_i = particles.p_y[i];
    const float v_x_i = particles.v_x[i];
    const float v_y_i = particles.v_y[i];
    const float i_frac = p_over_rho2[i];
    const float eps = 0.01f * h * h;

    const int start = i * MAX_NEIGHBORS;
    for (int curr = 0; curr < neighbors.counts[i]; curr++) {
        const int j = neighbors.neighbors[start + curr];

        const float d_p_x = p_x_i - particles.p_x[j];
        const float d_p_y = p_y_i - particles.p_y[j];
        const float d_v_x = v_x_i - particles.v_x[j];
        const float d_v_y = v_y_i - particles.v_y[j];
        const float dot_v_p = d_v_x*d_p_x + d_v_y*d_p_y;
        const float dot_p_p = d_p_x*d_p_x + d_p_y*d_p_y;

        const glm::vec2 kernel_deriv = sph::kernels::cubic_spline_2D_deriv(d_p_x, d_p_y, dot_p_p, kernel_const);

        const float m_j = particles.m[j];

        const float j_frac = (particles.is_bound[j])? i_frac : p_over_rho2[j];
        const float j_m_over_rho = (particles.is_bound[j])? particles.m[j] / particles.rho[i] : m_over_rho[j];

        const float p_sum_no_deriv = m_j * (i_frac + j_frac);
        const float v_sum_no_deriv = j_m_over_rho * dot_v_p / (dot_p_p + eps);

        p_sum_x += p_sum_no_deriv * kernel_deriv.x;
        p_sum_y += p_sum_no_deriv * kernel_deriv.y;
        v_sum_x += v_sum_no_deriv * kernel_deriv.x;
        v_sum_y += v_sum_no_deriv * kernel_deriv.y;
    }

    return -glm::vec2(p_sum_x, p_sum_y) + 2.0f * nu * glm::vec2(v_sum_x, v_sum_y);
}

glm::vec2 FluidSolver::gravity_acceleration(const int i) const {
    return g / particles.m[i];
}

void FluidSolver::step(Grid &grid) {
    int max_neighbor_overflow_count = 0;
    grid.populate_cells();

    // Precomputed temporary values
    auto p_over_rho2 = std::vector<float>(particles.count);
    auto m_over_rho = std::vector<float>(particles.count);
    const sph::kernels::kernel_constants kernel_const(h);

    #pragma omp parallel
    {
        #pragma omp for schedule(static) reduction(max:max_neighbor_overflow_count)
        for (int i = 0; i < particles.count; i++) {
            grid.calculate_neighbors(i, h, neighbors, max_neighbor_overflow_count);
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < particles.count; i++) {
            if (particles.is_bound[i]) continue;
            const float rho_i = density_explicit(i, kernel_const);
            particles.rho[i] = rho_i;
            const float p_i = pressure(i);
            particles.p[i] = p_i;

            p_over_rho2[i] = p_i / (rho_i * rho_i);
            m_over_rho[i] = particles.m[i] / rho_i;
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < particles.count; i++) {
            if (particles.is_bound[i]) continue;
            glm::vec2 acc = {0,0};
            acc += g;
            acc += combined_acceleration(i, kernel_const, p_over_rho2, m_over_rho);

            particles.a_x[i] = acc.x;
            particles.a_y[i] = acc.y;
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < particles.count; i++) {
            if (particles.is_bound[i]) continue;

            const glm::vec2 vel = sph::integrators::euler_cromer_vel_step({particles.v_x[i], particles.v_y[i]}, {particles.a_x[i], particles.a_y[i]}, dt);

            particles.v_x[i] = vel.x;
            particles.v_y[i] = vel.y;

            const glm::vec2 pos = sph::integrators::euler_cromer_pos_step({particles.p_x[i], particles.p_y[i]}, {particles.v_x[i], particles.v_y[i]}, dt);

            particles.p_x[i] = pos.x;
            particles.p_y[i] = pos.y;
        }
    }
    if (max_neighbor_overflow_count > 0) {
        std::cout << "Neighbor overflow count: " << max_neighbor_overflow_count << std::endl;
    }

    // Doing the removal process here to avoid data races in multithreading
    std::vector<int> temp_remove_indices;

    float max_v2 = 0.0f;
    for (int i = 0; i < particles.count; i++) {
        if (grid.get_cell_index(particles.p_x[i], particles.p_y[i]) == -1) {
            temp_remove_indices.push_back(i);
        }
        const float v_x = particles.v_x[i];
        const float v_y = particles.v_y[i];
        const float v2 = v_x*v_x + v_y*v_y;
        if (v2 > max_v2) max_v2 = v2;
    }
    max_v = std::sqrt(max_v2);

    for (auto it = temp_remove_indices.rbegin(); it != temp_remove_indices.rend(); ++it) {
        particles.remove(*it);
    }
}

void FluidSolver::clean_particles() {
    particles.clear();
    neighbors.clear();
}

std::vector<int> FluidSolver::get_neighbors(const int i) const {
    std::vector<int> neighbors_i;

    const int start = i * MAX_NEIGHBORS;
    for (int curr = 0; curr < neighbors.counts[i]; curr++) {
        const int j = neighbors.neighbors[start + curr];

        neighbors_i.push_back(j);
    }

    return neighbors_i;
}

float FluidSolver::get_cfl_lambda() const {
    return dt * max_v / h;
}
