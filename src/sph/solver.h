#ifndef SPH_FLUID_SOLVER_SOLVER_H
#define SPH_FLUID_SOLVER_SOLVER_H
#include <vector>
#include "data_structure.h"
#include "particle.h"
#include "../app/object.h"

namespace sph::kernels {
    struct kernel_constants;
}

namespace stats {
    struct Snapshot;
}

class FluidSolver {
public:
    FluidSolver(float dt, float h, float k, float nu, glm::vec2 g, float gamma_1, float gamma_2, float gamma_coh, float gamma_curv, float gamma_ad);
    void step(Grid &grid, std::vector<ParticleRemoval> &removals);
    void add_particle(glm::vec2 o, glm::vec3 color, float rho_0, bool is_fixed = false);
    void add_particle_grid(glm::ivec2 N, glm::vec2 o, glm::vec3 color, float rho_0, bool is_fixed = false, float r = 0.0f);
    void add_particle_grid(const Particles &p_other);
    void update_nu_boundaries(Grid &grid);
    void clean_particles();
    [[nodiscard]] int get_num_particles() const {return particles.count;}
    [[nodiscard]] std::vector<int> get_neighbors(int i) const;
    [[nodiscard]] float get_cfl_lambda() const;
    [[nodiscard]] float get_particle_mass(float rho_0) const;
    [[nodiscard]] float get_boundary_volume(int i, const sph::kernels::kernel_constants &kernel_const) const;

    Particles particles;
    NeighborList neighbors;

    float dt;
    float h;
    float k;
    float nu;
    float max_v;
    float gamma_1; // density coefficient for missing boundary samples
    float gamma_2; // pressure acceleration coefficient
    /* A ratio of 100 to 1 seems to work very nicely for cohesion and curvature */
    /* gamma_ad should be about 10x gamma_coh, interesting setup 1000, 10, 10000*/
    float gamma_coh; // surface tension cohesion coefficient
    float gamma_curv; // surface tension curvature coefficient
    float gamma_ad; // adhesion coefficient
    glm::vec2 g{};

private:
    [[nodiscard]] float density_explicit(int i, const sph::kernels::kernel_constants &kernel_const) const;
    [[nodiscard]] float pressure(int i) const;
    [[nodiscard]] glm::vec2 combined_acceleration(int i, const sph::kernels::kernel_constants &kernel_const, const std::vector<float> &p_over_theta2, const std::vector<float> &m_over_rho) const;
    [[nodiscard]] glm::vec2 gravity_acceleration(int i) const;
    [[nodiscard]] glm::vec2 calculate_st_n(int i, const std::vector<float> &m_over_rho, const sph::kernels::kernel_constants &kernel_const) const;
    [[nodiscard]] glm::vec2 st_cohesion_curvature_adhesion_acceleration(int i, const std::vector<glm::vec2> &n) const;
};

#endif //SPH_FLUID_SOLVER_SOLVER_H
