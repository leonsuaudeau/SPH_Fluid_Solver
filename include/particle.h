#ifndef SPH_FLUID_SOLVER_PARTICLE_H
#define SPH_FLUID_SOLVER_PARTICLE_H

struct Particle2D {
    float x, y;
    float vx, vy;
    float mass;
};

struct Particle3D {
    float x, y, z;
    float vx, vy, vz;
    float mass;
};

#endif //SPH_FLUID_SOLVER_PARTICLE_H
