#ifndef SPH_FLUID_SOLVER_MATH_H
#define SPH_FLUID_SOLVER_MATH_H
#include <vector>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "particle.h"

glm::vec2 inline euler_cromer_pos_step(const glm::vec2 x_t, const glm::vec2 v_t_next, const float dt) {
    return x_t + v_t_next * dt;
}
glm::vec2 inline euler_cromer_vel_step(const glm::vec2 v_t, const glm::vec2 a_t, const float dt) {
    return v_t + a_t * dt;
}

float inline kernel_func_2D(const glm::vec2 x_i, const glm::vec2 x_j, const float h) {
    const float q = glm::length(x_i - x_j) / h;
    if (q >= 2.0f) return 0.0f;

    const float alpha = 5.0f / (14.0f * 3.14159265359f * h * h);
    float w_q = (2.0f - q) * ((2.0f - q) * (2.0f - q));
    if (q < 1.0f) {
        w_q -= 4.0f * ((1.0f - q) * (1.0f - q) * (1.0f - q));
    }
    return alpha * w_q;
}

glm::vec2 inline kernel_deriv_2D(const glm::vec2 x_i, const glm::vec2 x_j, const float h) {
    const float q = glm::length(x_i - x_j) / h;
    if (q >= 2.0f) return glm::vec2(0.0f);

    const float alpha = 5.0f / (14.0f * 3.14159265359f * h * h);
    float w_q = -3.0f * ((2.0f - q) * (2.0f - q));
    if (q < 1.0f) {
        w_q += 12.0f * ((1.0f - q) * (1.0f - q));
    }
    return alpha * w_q * ((x_i - x_j) / (glm::length(x_i - x_j) * h));
}

float inline density_explicit(const Particle2D &p_i, const std::vector<Particle2D> &neighbors, const float h) {
    float sum = 0;
    for (const auto &p_j : neighbors) {
        sum += p_j.mass * kernel_func_2D(p_i.pos, p_j.pos, h);
    }
    return sum;
}

float inline pressure(const Particle2D &p_i, const float k, const float rho_0) {
    return  glm::max(k * (p_i.density / rho_0) - 1, 0.0f);
}

glm::vec2 inline get_pressure_accel(const Particle2D &p_i, const std::vector<Particle2D> &neighbors, const float h) {
    auto sum = glm::vec2(0);
    const float i_frac = p_i.pressure / (p_i.density * p_i.density);
    for (const auto &p_j : neighbors) {
        sum += p_j.mass * (i_frac + p_j.pressure / (p_j.density * p_j.density)) * kernel_deriv_2D(p_i.pos, p_j.pos, h);
    }
    return -sum;
}

glm::vec2 inline get_viscosity_accel(const Particle2D &p_i, const std::vector<Particle2D> &neighbors, const float h, const float nu) {
    auto sum = glm::vec2(0);
    for (const auto &p_j : neighbors) {
        glm::vec2 x_ij = p_j.pos - p_i.pos;
        sum += p_j.mass / p_j.density * ((p_j.vel - p_i.vel) * x_ij) / (x_ij * x_ij + 0.01f * h * h) * kernel_deriv_2D(p_i.pos, p_j.pos, h);
    }
    return 2 * nu * sum;
}

#endif //SPH_FLUID_SOLVER_MATH_H
