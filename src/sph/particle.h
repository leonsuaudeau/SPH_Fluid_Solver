#ifndef SPH_FLUID_SOLVER_PARTICLE_H
#define SPH_FLUID_SOLVER_PARTICLE_H
#include <vector>
#include <glm/glm.hpp>

struct LegacyParticle2D {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec2 acc;
    float mass;
    float pressure;
    float density;
    glm::vec3 color;
    bool is_fixed = false;
};

struct Particle2D {
    glm::vec2 pos;
    glm::vec2 vel;
    float mass;
    float density;
    float rest_density;
    glm::vec3 color;
    bool is_fixed = false;
};

constexpr int MAX_PARTICLES = 1000000;

struct Particles {
    explicit Particles(int max_particles = MAX_PARTICLES);
    int add(glm::vec2 pos, glm::vec2 vel, float mass, float density, float rest_density, bool is_boundary, float boundary_volume = 0.0f, glm::vec3 color = glm::vec3(0.0f));
    void combine(const Particles &other);
    void remove(int i);
    void clear();

    int count = 0;
    int capacity = 0;

    std::vector<float> p_x;
    std::vector<float> p_y;

    std::vector<float> v_x;
    std::vector<float> v_y;

    std::vector<float> a_x;
    std::vector<float> a_y;

    std::vector<float> m;
    std::vector<float> p;
    std::vector<float> rho;
    std::vector<float> rho_0;

    std::vector<float> col_r;
    std::vector<float> col_g;
    std::vector<float> col_b;

    std::vector<u_int8_t> is_bound;
    std::vector<float> bound_vol;
};

inline Particles::Particles(const int max_particles) {
    capacity = max_particles;

    p_x.resize(capacity);
    p_y.resize(capacity);
    v_x.resize(capacity);
    v_y.resize(capacity);
    a_x.resize(capacity);
    a_y.resize(capacity);
    m.resize(capacity);
    p.resize(capacity);
    rho.resize(capacity);
    rho_0.resize(capacity);
    col_r.resize(capacity);
    col_g.resize(capacity);
    col_b.resize(capacity);
    is_bound.resize(capacity);
    bound_vol.resize(capacity);
}

inline int Particles::add(
    const glm::vec2 pos, const glm::vec2 vel, const float mass,
    const float density, const float rest_density, const bool is_boundary, const float boundary_volume, const glm::vec3 color)
{
    if (count >= capacity) {
        return 1;
    }

    const int i = count++;

    p_x[i] = pos.x;
    p_y[i] = pos.y;
    v_x[i] = vel.x;
    v_y[i] = vel.y;
    a_x[i] = 0.0f;
    a_y[i] = 0.0f;
    m[i] = mass;
    p[i] = 0.0f;
    rho[i] = density;
    rho_0[i] = rest_density;
    col_r[i] = color.r;
    col_g[i] = color.g;
    col_b[i] = color.b;
    is_bound[i] = is_boundary;
    bound_vol[i] = boundary_volume;

    return 0;
}

inline void Particles::remove(const int i) {
    const int last = count - 1;
    if (i != last) {
        p_x[i] = p_x[last];
        p_y[i] = p_y[last];
        v_x[i] = v_x[last];
        v_y[i] = v_y[last];
        a_x[i] = a_x[last];
        a_y[i] = a_y[last];
        m[i] = m[last];
        p[i] = p[last];
        rho[i] = rho[last];
        rho_0[i] = rho_0[last];
        col_r[i] = col_r[last];
        col_g[i] = col_g[last];
        col_b[i] = col_b[last];
        is_bound[i] = is_bound[last];
        bound_vol[i] = bound_vol[last];
    }
    count--;
}

inline void Particles::clear() {
    count = 0;
}

inline void Particles::combine(const Particles &other) {
    const int n = glm::min(capacity - count, other.count);

    for (int k = 0; k < n; k++) {
        const int i = count + k;
        p_x[i] = other.p_x[k];
        p_y[i] = other.p_y[k];
        v_x[i] = other.v_x[k];
        v_y[i] = other.v_y[k];
        a_x[i] = other.a_x[k];
        a_y[i] = other.a_y[k];
        m[i] = other.m[k];
        p[i] = other.p[k];
        rho[i] = other.rho[k];
        rho_0[i] = other.rho_0[k];
        col_r[i] = other.col_r[k];
        col_g[i] = other.col_g[k];
        col_b[i] = other.col_b[k];
        is_bound[i] = other.is_bound[k];
        bound_vol[i] = other.bound_vol[k];
    }
    count += n;
}

#endif //SPH_FLUID_SOLVER_PARTICLE_H
