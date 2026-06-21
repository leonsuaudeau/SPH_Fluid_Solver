#ifndef SPH_FLUID_SOLVER_PARTICLE_H
#define SPH_FLUID_SOLVER_PARTICLE_H
#include <vector>
#include <glm/glm.hpp>

struct Particle2D {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec2 acc;
    float mass;
    float pressure;
    float density;
    glm::vec3 color; // TODO: this is just for now
    bool is_fixed = false; // TODO: remove this later
};

constexpr int MAX_PARTICLES = 100000;

struct Particles {
    explicit Particles(int max_particles = MAX_PARTICLES);
    int add(glm::vec2 pos, glm::vec2 vel, glm::vec2 acc, float mass, float density, bool is_boundary, glm::vec3 color = glm::vec3(0.0f));
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

    std::vector<float> col_r;
    std::vector<float> col_g;
    std::vector<float> col_b;

    std::vector<u_int8_t> is_bound;
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
    col_r.resize(capacity);
    col_g.resize(capacity);
    col_b.resize(capacity);
    is_bound.resize(capacity);
}

inline int Particles::add(
    const glm::vec2 pos, const glm::vec2 vel, const glm::vec2 acc, const float mass,
    const float density, const bool is_boundary, const glm::vec3 color)
{
    if (count >= capacity) {
        return 1;
    }

    const int i = count++;

    p_x[i] = pos.x;
    p_y[i] = pos.y;
    v_x[i] = vel.x;
    v_y[i] = vel.y;
    a_x[i] = acc.x;
    a_y[i] = acc.y;
    m[i] = mass;
    p[i] = 0.0f;
    rho[i] = density;
    col_r[i] = color.r;
    col_g[i] = color.g;
    col_b[i] = color.b;
    is_bound[i] = is_boundary;

    return 0;
}

inline void Particles::remove(int i) {
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
        col_r[i] = col_r[last];
        col_g[i] = col_g[last];
        col_b[i] = col_b[last];
        is_bound[i] = is_bound[last];
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
        col_r[i] = other.col_r[k];
        col_g[i] = other.col_g[k];
        col_b[i] = other.col_b[k];
        is_bound[i] = other.is_bound[k];
    }
    count += n;
}

#endif //SPH_FLUID_SOLVER_PARTICLE_H
