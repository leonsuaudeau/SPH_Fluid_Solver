#ifndef SPH_FLUID_SOLVER_SOLVER_H
#define SPH_FLUID_SOLVER_SOLVER_H
#include <vector>
#include "particle.h"

std::vector<std::vector<Particle2D>> get_neighbors(const std::vector<Particle2D> &particles, float h);

std::vector<Particle2D> create_uniform_grid(glm::ivec2 particle_count, glm::vec2 grid_origin, float m_i, float rho_0, float dx, glm::vec3 color, bool is_fixed = false);

void fluid_solver_iteration(std::vector<Particle2D> &particles, float dt, float h, float rho_0, float k);

#endif //SPH_FLUID_SOLVER_SOLVER_H
