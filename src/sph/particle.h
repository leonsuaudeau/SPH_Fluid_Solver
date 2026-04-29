#ifndef SPH_FLUID_SOLVER_PARTICLE_H
#define SPH_FLUID_SOLVER_PARTICLE_H
#include <glm/glm.hpp>

struct Particle2D {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec2 acc;
    float mass;
    float pressure;
    float density;
};

#endif //SPH_FLUID_SOLVER_PARTICLE_H
