#ifndef SPH_FLUID_SOLVER_SPH_KERNEL_H
#define SPH_FLUID_SOLVER_SPH_KERNEL_H
#include <glm/glm.hpp>

namespace sph::kernels {
    /* Exact kernel formulations taken from lecture slides:
     * Matthias Teschner, Simulation in Computer Graphics, University of Freiburg */

    float inline cubic_spline_2D(const float p_x_i, const float p_y_i, const float p_x_j, const float p_y_j, const float h) {
        glm::vec2 x_i = glm::vec2(p_x_i, p_y_i);
        glm::vec2 x_j = glm::vec2(p_x_j, p_y_j); // TODO: replace the formulae here
        const float d = glm::length(x_i - x_j) / h;
        const float t1 = glm::max(1.0f - d, 0.0f);
        const float t2 = glm::max(2.0f - d, 0.0f);
        const float alpha = 5.0f / (14.0f * 3.14159265359f * h * h);
        return alpha * (t2 * t2 * t2 - 4 * t1 * t1 * t1);
    }

    glm::vec2 inline cubic_spline_2D_deriv(const float p_x_i, const float p_y_i, const float p_x_j, const float p_y_j, const float h) {
        glm::vec2 x_i = glm::vec2(p_x_i, p_y_i);
        glm::vec2 x_j = glm::vec2(p_x_j, p_y_j);
        const float d = glm::length(x_i - x_j) / h;
        if (d < 1e-6f) return glm::vec2(0);
        const float t1 = glm::max(1.0f - d, 0.0f);
        const float t2 = glm::max(2.0f - d, 0.0f);
        const float alpha = 5.0f / (14.0f * 3.14159265359f * h * h);
        return alpha * (x_i - x_j) / (d * h * h) * (-3 * t2 * t2 + 12 * t1 * t1);
    }

}

#endif //SPH_FLUID_SOLVER_SPH_KERNEL_H
