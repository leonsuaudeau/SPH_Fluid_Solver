#include "solver.h"
#include "sph_math.h"

std::vector<std::vector<Particle2D>> get_neighbors(const std::vector<Particle2D> &particles, const float h) {
    std::vector<std::vector<Particle2D>> neighbors{};

    for (int i = 0; i < particles.size(); i++) {
        neighbors.emplace_back();
        for (int j = 0; j < particles.size(); j++) {
            glm::vec2 p_ij = particles[j].pos - particles[i].pos;
            if (length(p_ij) <= 2 * h) {
                neighbors[i].push_back(particles[j]);
            }
        }
    }
    return neighbors;
}

std::vector<Particle2D> create_uniform_grid(glm::ivec2 particle_count, glm::vec2 grid_origin, float m_i, float rho_0, float dx, glm::vec3 color, bool is_fixed) {
    std::vector<Particle2D> particles{};
    for (int y = 0; y < particle_count.y; y++ ) {
        for (int x = 0; x < particle_count.x; x++) {
            const glm::vec2 pos = grid_origin + glm::vec2(x, y) * dx;
            particles.emplace_back(Particle2D(pos, glm::vec2(0), glm::vec2(0), m_i, 0, rho_0, color, is_fixed));
        }
    }
    return particles;
}

void fluid_solver_iteration(std::vector<Particle2D> &particles, const float dt, const float h, const float rho_0, const float k) {
    const auto neighbors = get_neighbors(particles, h);

    for (int i = 0; i < particles.size(); i++) {
        particles[i].density = density_explicit(particles[i], neighbors[i], h);
        particles[i].pressure = pressure(particles[i], k, rho_0);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.acc = glm::vec2(0);
        p_i.acc += glm::vec2(0, -9.81f);
        p_i.acc += get_viscosity_accel(particles[i], neighbors[i], h, 0.000001f);
        p_i.acc += get_pressure_accel(particles[i], neighbors[i], h);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.vel = euler_cromer_vel_step(p_i.vel, p_i.acc, dt);
        p_i.pos = euler_cromer_pos_step(p_i.pos, p_i.vel, dt);
    }
}