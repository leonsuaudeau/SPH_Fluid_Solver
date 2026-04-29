#ifndef SPH_FLUID_SOLVER_SOLVER_H
#define SPH_FLUID_SOLVER_SOLVER_H
#include <vector>
#include "particle.h"

void fluid_solver_iteration(std::vector<Particle2D> &particles, float dt, float h);

#endif //SPH_FLUID_SOLVER_SOLVER_H
