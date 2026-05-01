#include "solver.h"
#include "sph_integrators.h"
#include "sph_kernel.h"

FluidSolver::FluidSolver(const float h, const float rho_0, const float k, const float nu, const glm::vec2 g) {
    this->h = h;
    this->rho_0 = rho_0;
    this->k = k;
    this->nu = nu;
    this->g = g;
}

void FluidSolver::add_particle(const glm::vec2 o, const glm::vec3 color, const bool is_fixed) {
    particles.emplace_back(Particle2D(o, glm::vec2(0), glm::vec2(0), particle_mass(), 0, rho_0, color, is_fixed));
}

void FluidSolver::add_particle_grid(const glm::ivec2 N, const glm::vec2 o, const glm::vec3 color, const bool is_fixed) {
    const float m_i = particle_mass();
    for (int y = 0; y < N.y; y++ ) {
        for (int x = 0; x < N.x; x++) {
            const glm::vec2 pos = o + glm::vec2(x, y) * h;
            particles.emplace_back(Particle2D(pos, glm::vec2(0), glm::vec2(0), m_i, 0, rho_0, color, is_fixed));
        }
    }
}

float FluidSolver::particle_mass() const {
    return h * h * rho_0;
}

float FluidSolver::density_explicit(const int i) const {
    float sum = 0;
    const Particle2D &p_i = particles[i];
    for (const auto j: neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        sum += p_j.mass * sph::kernels::cubic_spline_2D(p_i.pos, p_j.pos, h);
    }
    return sum;
}

float FluidSolver::pressure(const int i) const {
    const Particle2D &p_i = particles[i];
    return  glm::max(k * (p_i.density / rho_0 - 1), 0.0f);
}

glm::vec2 FluidSolver::pressure_acceleration(const int i) const {
    const Particle2D &p_i = particles[i];
    auto sum = glm::vec2(0);
    const float i_frac = p_i.pressure / (p_i.density * p_i.density);
    for (const auto j : neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        sum += p_j.mass * (i_frac + p_j.pressure / (p_j.density * p_j.density)) * sph::kernels::cubic_spline_2D_deriv(p_i.pos, p_j.pos, h);
    }
    return -sum;
}

glm::vec2 FluidSolver::viscosity_acceleration(const int i) const {
    const Particle2D &p_i = particles[i];
    auto sum = glm::vec2(0);
    for (const auto j : neighbor_indices[i]) {
        const Particle2D &p_j = particles[j];
        glm::vec2 x_ij = p_j.pos - p_i.pos;
        sum += p_j.mass / p_j.density * ((p_j.vel - p_i.vel) * x_ij) / (x_ij * x_ij + 0.01f * h * h) * sph::kernels::cubic_spline_2D_deriv(p_i.pos, p_j.pos, h);
    }
    return 2 * nu * sum;
}

glm::vec2 FluidSolver::gravity_acceleration(int i) const {
    const Particle2D &p_i = particles[i];
    return g / p_i.mass;
}

void FluidSolver::update_neighbors() {
    neighbor_indices.clear();
    for (int i = 0; i < particles.size(); i++) {
        neighbor_indices.emplace_back();
        for (int j = 0; j < particles.size(); j++) {
            if (const float d = length(particles[j].pos - particles[i].pos); d <= 2 * h) {
                neighbor_indices[i].push_back(j);
            }
        }
    }
}

void FluidSolver::step(const float dt) {
    update_neighbors();

    for (int i = 0; i < particles.size(); i++) {
        particles[i].density = density_explicit(i);
        particles[i].pressure = pressure(i);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.acc = glm::vec2(0);
        p_i.acc += gravity_acceleration(i);
        p_i.acc += viscosity_acceleration(i);
        p_i.acc += pressure_acceleration(i);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle2D &p_i = particles[i];
        if (p_i.is_fixed) continue;
        p_i.vel = sph::integrators::euler_cromer_vel_step(p_i.vel, p_i.acc, dt);
        p_i.pos = sph::integrators::euler_cromer_pos_step(p_i.pos, p_i.vel, dt);
    }
}

void FluidSolver::clean_particles() {
    particles.clear();
    neighbor_indices.clear();
}

std::vector<Particle2D> FluidSolver::get_neighbors(const int i) {
    std::vector<Particle2D> neighbors{};
    for (const auto j : neighbor_indices[i]) {
        neighbors.emplace_back(particles[j]);
    }
    return neighbors;
}
