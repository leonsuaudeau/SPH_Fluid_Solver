#include "solver.h"
#include <execution>
#include <algorithm>
#include <chrono>
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
    for (const auto j: neighbor_indices[i]) {
        const float d_p_x = p_x_i - particles.p_x[j];
        const float d_p_y = p_y_i - particles.p_y[j];
        const float dot_p_p = d_p_x*d_p_x + d_p_y*d_p_y;
        sum += particles.m[j] * sph::kernels::cubic_spline_2D(dot_p_p, kernel_const);
    }
    return sum;
}

float FluidSolver::pressure(const int i) const {
    return  glm::max(k * (particles.rho[i] / rho_0 - 1), 0.0f);
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

    for (const auto j : neighbor_indices[i]) {
        const float d_p_x = p_x_i - particles.p_x[j];
        const float d_p_y = p_y_i - particles.p_y[j];
        const float d_v_x = v_x_i - particles.v_x[j];
        const float d_v_y = v_y_i - particles.v_y[j];
        const float dot_v_p = d_v_x*d_p_x + d_v_y*d_p_y;
        const float dot_p_p = d_p_x*d_p_x + d_p_y*d_p_y;

        const glm::vec2 kernel_deriv = sph::kernels::cubic_spline_2D_deriv(d_p_x, d_p_y, dot_p_p, kernel_const);

        const float m_j = particles.m[j];
        const float p_sum_no_deriv = m_j * (i_frac + p_over_rho2[j]);
        const float v_sum_no_deriv = m_over_rho[j] * dot_v_p / (dot_p_p + eps);

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

void FluidSolver::update_neighbors() {
    const float radius2 = 4.0f * h * h + 0.0001f;
    neighbor_indices.clear();
    for (int i = 0; i < particles.count; i++) {
        neighbor_indices.emplace_back();
        for (int j = 0; j < particles.count; j++) {
            const float d_x = particles.p_x[j] - particles.p_x[i];
            const float d_y = particles.p_y[j] - particles.p_y[i];
            if (const float d2 = d_x*d_x + d_y*d_y; d2 < radius2) {
                neighbor_indices[i].push_back(j);
            }
        }
    }
}

void FluidSolver::update_neighbors_parallel() {
    const float radius2 = 4.0f * h * h + 0.0001f;
    int N = get_num_particles();
    neighbor_indices = std::vector<std::vector<int>>(N);
    std::vector<int> indices(N);
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](int i) {
        std::vector<int> local;
        for (int j = 0; j < particles.count; j++) {
            const float d_x = particles.p_x[j] - particles.p_x[i];
            const float d_y = particles.p_y[j] - particles.p_y[i];
            if (const float d2 = d_x*d_x + d_y*d_y; d2 < radius2) {
                local.push_back(j);
            }
        }
        neighbor_indices[i] = std::move(local);
    });
}

void FluidSolver::step(Grid &grid) {
    auto t0 = std::chrono::system_clock::now();

    float max_v2 = 0.0f;

    grid.populate_cells();

    auto t1 = std::chrono::system_clock::now();

    grid.calculate_neighbors(h, neighbor_indices);

    auto t2 = std::chrono::system_clock::now();

    //update_neighbors();
    //update_neighbors_parallel();

    // Precomputed temporary values
    auto p_over_rho2 = std::vector<float>(particles.count);
    auto m_over_rho = std::vector<float>(particles.count);
    sph::kernels::kernel_constants kernel_const(h);

    for (int i = 0; i < particles.count; i++) {
        const float rho_i = density_explicit(i, kernel_const);
        particles.rho[i] = rho_i;
        const float p_i = pressure(i);
        particles.p[i] = p_i;

        p_over_rho2[i] = p_i / (rho_i * rho_i);
        m_over_rho[i] = particles.m[i] / rho_i;
    }

    auto t3 = std::chrono::system_clock::now();

    for (int i = 0; i < particles.count; i++) {
        glm::vec2 acc = {0,0};
        acc += gravity_acceleration(i);
        acc += combined_acceleration(i, kernel_const, p_over_rho2, m_over_rho);

        particles.a_x[i] = acc.x;
        particles.a_y[i] = acc.y;
    }

    auto t4 = std::chrono::system_clock::now();

    for (int i = 0; i < particles.count; i++) {
        if (particles.is_bound[i]) continue;

        const glm::vec2 vel = sph::integrators::euler_cromer_vel_step({particles.v_x[i], particles.v_y[i]}, {particles.a_x[i], particles.a_y[i]}, dt);

        particles.v_x[i] = vel.x;
        particles.v_y[i] = vel.y;

        if (const float v2 = vel.x*vel.x + vel.y*vel.y; v2 > max_v2) max_v2 = v2; // update for cfl

        const glm::vec2 pos = sph::integrators::euler_cromer_pos_step({particles.p_x[i], particles.p_y[i]}, {particles.v_x[i], particles.v_y[i]}, dt);

        particles.p_x[i] = pos.x;
        particles.p_y[i] = pos.y;
    }

    auto t5 = std::chrono::system_clock::now();

    max_v = std::sqrt(max_v2);

    // Doing the removal process here to avoid data races in multithreading
    std::vector<int> temp_remove_indices;

    for (int i = 0; i < particles.count; i++) {
        if (grid.get_cell_index(particles.p_x[i], particles.p_y[i]) == -1) {
            temp_remove_indices.push_back(i);
        }
    }

    for (const int i : temp_remove_indices) {
        particles.remove(i);
    }

    auto t6 = std::chrono::system_clock::now();


    std::cout << "Reset Grid: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0f <<
        "ms; Neighbor Search: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0f <<
            "ms; Density&Pressure: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1000.0f <<
                "ms; Accelerations: " << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1000.0f <<
                    "ms; Integration: " << std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count() / 1000.0f <<
                        "ms; Removal: " << std::chrono::duration_cast<std::chrono::microseconds>(t6 - t5).count() / 1000.0f << "ms" << "\n";


}

void FluidSolver::clean_particles() {
    particles.clear();
    neighbor_indices.clear();
}

std::vector<int> FluidSolver::get_neighbors(const int i) {
    return neighbor_indices[i];
}

float FluidSolver::get_cfl_lambda() const {
    return dt * max_v / h;
}
