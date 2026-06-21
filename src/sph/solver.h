#ifndef SPH_FLUID_SOLVER_SOLVER_H
#define SPH_FLUID_SOLVER_SOLVER_H
#include <vector>
#include "data_structure.h"
#include "particle.h"

namespace sph::kernels {
    struct kernel_constants;
}

namespace stats {
    struct Snapshot;
}

class FluidSolver {
public:
    FluidSolver(float dt, float h, float rho_0, float k, float nu, glm::vec2 g);
    void step(Grid &grid);
    void add_particle(glm::vec2 o, glm::vec3 color, bool is_fixed = false);
    void add_particle_grid(glm::ivec2 N, glm::vec2 o, glm::vec3 color, bool is_fixed = false, float r = 0.0f);
    void add_particle_grid(const Particles &p_other);
    void clean_particles();
    [[nodiscard]] int get_num_particles() const {return particles.count;}
    [[nodiscard]] float get_particle_density(int i) const {return particles.rho[i];}
    std::vector<int> get_neighbors(int i);
    [[nodiscard]] float get_cfl_lambda() const;
    [[nodiscard]] float get_particle_mass() const;

    Particles particles;
    std::vector<std::vector<int>> neighbor_indices{};
    float dt;
    float h;
    float rho_0;
    float k;
    float nu;
    float max_v;
    glm::vec2 g{};

private:
    [[nodiscard]] float density_explicit(int i, sph::kernels::kernel_constants kernel_const) const;
    [[nodiscard]] float pressure(int i) const;
    [[nodiscard]] glm::vec2 combined_acceleration(int i, sph::kernels::kernel_constants kernel_const, const std::vector<float> &p_over_rho2, const std::vector<float> &m_over_rho) const;
    [[nodiscard]] glm::vec2 gravity_acceleration(int i) const;
    void update_neighbors();
    void update_neighbors_parallel();
};

#endif //SPH_FLUID_SOLVER_SOLVER_H
