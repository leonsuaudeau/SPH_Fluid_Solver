#ifndef SPH_FLUID_SOLVER_MATH_H
#define SPH_FLUID_SOLVER_MATH_H
#include <vector>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/norm.hpp>

#include "particle.h"

glm::vec2 inline euler_cromer_pos_step(const glm::vec2 x_t, const glm::vec2 v_t_next, const float h) {
    return x_t + v_t_next * h;
}
glm::vec2 inline euler_cromer_vel_step(const glm::vec2 v_t, const glm::vec2 a_t, const float h) {
    return v_t * a_t *  h;
}

float inline kernel_func_2D(const glm::vec2 x_i, const glm::vec2 x_j, const float h) {
    const float q = glm::length(x_j - x_i) / h;
    if (q >= 2) return 0;

    const float alpha = 5.0f / (14.0f * 3.14159265359f * h * h);
    float w_q = 0;
    float base = 2 - q;
    w_q += base * base * base;
    if (q < 1) {
        base = 1 - q;
        w_q -= 4 * base * base * base;
    }
    return alpha * w_q;
}

glm::vec2 inline kernel_deriv_2D(const glm::vec2 x_i, const glm::vec2 x_j, const float h) {
    const float q = glm::length(x_j - x_i) / h;
    if (q >= 2) return glm::vec3(0);

    const glm::vec2 alpha = 5.0f * (x_j - x_i) / 14.0f * 3.14159265359f * h * h * h * glm::length(x_j - x_i);
    auto w_q = glm::vec2(0);
    float base = 2 - q;
    w_q -= 3 * base * base;
    if (q < 1) {
        base = 1 - q;
        w_q += 12 * base * base;
    }
    return alpha * w_q;
}

void inline update_density_explicit(Particle2D &p_i, const std::vector<Particle2D> &particles, const float h) {
    float sum = 0;
    for (const auto &p_j : particles) {
        sum += p_j.mass * kernel_func_2D(p_i.pos, p_j.pos, h);
    }
    p_i.density = sum;
}

void inline update_pressure(Particle2D &p_i, const float k, const float rho_0) {
    p_i.pressure = k * (p_i.density / rho_0) - 1;
}

glm::vec2 inline get_pressure_accel(const Particle2D &p_i, const std::vector<Particle2D> &particles, const float h) {
    auto sum = glm::vec2(0);
    float p_i_press_by_density_squared = p_i.pressure / (p_i.density * p_i.density);
    for (const auto &p_j : particles) {
        sum += p_j.mass * (p_i_press_by_density_squared + p_j.pressure / (p_j.density * p_j.density) * kernel_deriv_2D(p_i.pos, p_j.pos, h));
    }
}

#endif //SPH_FLUID_SOLVER_MATH_H
