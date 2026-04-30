#ifndef SPH_FLUID_SOLVER_SPH_INTEGRATORS_H
#define SPH_FLUID_SOLVER_SPH_INTEGRATORS_H
#include <glm/glm.hpp>

namespace sph::integrators {

    glm::vec2 inline euler_cromer_pos_step(const glm::vec2 x_t, const glm::vec2 v_t_next, const float dt) {
        return x_t + v_t_next * dt;
    }
    glm::vec2 inline euler_cromer_vel_step(const glm::vec2 v_t, const glm::vec2 a_t, const float dt) {
        return v_t + a_t * dt;
    }

}

#endif //SPH_FLUID_SOLVER_SPH_INTEGRATORS_H
