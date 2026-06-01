#ifndef SPH_FLUID_SOLVER_SOLVER_H
#define SPH_FLUID_SOLVER_SOLVER_H
#include <vector>
#include "data_structure.h"
#include "particle.h"

namespace stats {
    struct Snapshot;
}

class FluidSolver {
public:
    FluidSolver(float dt, float h, float rho_0, float k, float nu, glm::vec2 g);
    void step(DataStructure::Grid &grid);
    void add_particle(glm::vec2 o, glm::vec3 color, bool is_fixed = false);
    void add_particle_grid(glm::ivec2 N, glm::vec2 o, glm::vec3 color, bool is_fixed = false, float r = 0.0f);
    void add_particle_grid(const std::vector<Particle2D> &p_other);
    void clean_particles();
    [[nodiscard]] int get_num_particles() const {return particles.size();}
    [[nodiscard]] float get_particle_density(int i) const {return particles[i].density;}
    std::vector<Particle2D> get_neighbors(int i);
    [[nodiscard]] float get_cfl_lambda() const;
    [[nodiscard]] float get_particle_mass() const;

    std::vector<Particle2D> particles{};
    std::vector<std::vector<int>> neighbor_indices{};
    float dt;
    float h;
    float rho_0;
    float k;
    float nu;
    float max_v;
    glm::vec2 g{};

private:
    [[nodiscard]] float density_explicit(int i) const;
    [[nodiscard]] float pressure(int i) const;
    [[nodiscard]] glm::vec2 pressure_acceleration(int i) const;
    [[nodiscard]] glm::vec2 viscosity_acceleration(int i) const;
    [[nodiscard]] glm::vec2 gravity_acceleration(int i) const;
    void update_neighbors();
    void update_neighbors_parallel();
};

#endif //SPH_FLUID_SOLVER_SOLVER_H
