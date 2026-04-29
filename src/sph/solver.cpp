#include "solver.h"
#include "sph_math.h"

void fluid_solver_iteration(std::vector<Particle2D> &particles, const float dt, const float h) {
    std::vector<std::vector<Particle2D>> neighborIndices{};

    for (int i = 0; i < particles.size(); i++) {
        neighborIndices.emplace_back();
        for (int j = 0; j < particles.size(); j++) {
            glm::vec2 p_ij = particles[j].pos - particles[i].pos;
            if (length(p_ij) <= 2 * h && i != j) {
                neighborIndices[i].push_back(particles[j]);
            }
        }
    }

    for (int i = 0; i < particles.size(); i++) {
        particles[i].density = density_explicit(particles[i], neighborIndices[i], h);
        particles[i].pressure = pressure(particles[i], 10, 1);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        p_i.acc = glm::vec2(0);
        //p_i.acc += glm::vec2(0, -9.81f);
        //p_i.acc += get_viscosity_accel(particles[i], neighborIndices[i], h, 0.000001f);
        //p_i.acc += get_pressure_accel(particles[i], neighborIndices[i], h);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        p_i.vel = euler_cromer_vel_step(p_i.vel, p_i.acc, dt);
        p_i.pos = euler_cromer_pos_step(p_i.pos, p_i.vel, dt);
    }
}