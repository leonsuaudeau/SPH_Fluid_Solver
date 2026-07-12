#ifndef SPH_FLUID_SOLVER_SPH_KERNEL_H
#define SPH_FLUID_SOLVER_SPH_KERNEL_H
#include <glm/glm.hpp>

namespace sph::kernels {
     /* Kernel formulations derived from Matthias Teschner, Simulation in Computer Graphics, University of Freiburg */

    struct kernel_constants {
        explicit kernel_constants(const float h) {
            const float h2 = h * h;
            alpha = 5.0f / (14.0f * 3.14159265359f * h2);
            deriv_factor = alpha / h2;
            inv_h = 1.0f / h;
        }
        float alpha;
        float deriv_factor;
        float inv_h;
    };

    float inline cubic_spline_2D(const float r2, const kernel_constants kernel_const) {
        const float d = glm::sqrt(r2) * kernel_const.inv_h;
        const float t1 = glm::max(1.0f - d, 0.0f);
        const float t2 = glm::max(2.0f - d, 0.0f);
        return kernel_const.alpha * (t2 * t2 * t2 - 4.0f * t1 * t1 * t1);
    }

    glm::vec2 inline cubic_spline_2D_deriv(const float d_x, const float d_y, const float r2, const kernel_constants kernel_const) {
        const float d = glm::sqrt(r2) * kernel_const.inv_h;
        if (d < 1e-12f) return {0.0f,0.0f};
        const float t1 = glm::max(1.0f - d, 0.0f);
        const float t2 = glm::max(2.0f - d, 0.0f);

        const float factor = kernel_const.deriv_factor  * (-3.0f * t2 * t2 + 12.0f * t1 * t1) / d;
        return {d_x * factor, d_y * factor};
    }

    float inline cohesion_spline_2D(const float r, const float h) {
        const float support = 2 * h;
        if (r <= 0.0f || r > support) return 0.0f;

        const float h_min_r = support - r;
        const float h_r_3_r_3 = h_min_r * h_min_r * h_min_r * r * r * r;

        const float support_2 = support * support;
        const float support_4 = support_2 * support_2;
        const float support_8 = support_4 * support_4;
        const float factor = 32.0f / (3.14159265359f * support_8);
        if (r > 0.5f * support) {
            return factor * h_r_3_r_3;
        }
        return factor * (2.0f * h_r_3_r_3 - support_4 * support_2 / 64.0f);
    }

}

#endif //SPH_FLUID_SOLVER_SPH_KERNEL_H
