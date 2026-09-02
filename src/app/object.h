#ifndef SPH_FLUID_SOLVER_OBJECT_H
#define SPH_FLUID_SOLVER_OBJECT_H
#include <algorithm>
#include "../sph/particle.h"
#include "glm/glm.hpp"

struct ParticleRemoval {
    int removed_index;
    int moved_from_index;
};

struct Object {
    Object(const std::vector<int> &particle_indices, glm::vec2 origin, glm::vec2 move_each_step = {0, 0}, float rotate_each_step = 0.0f);
    void move(float dx, float dy, Particles &particles);
    void rotate(float angle, Particles &particles) const;
    void update(Particles &particles);
    void on_particle_removed(int removed, int moved_from);

    std::vector<int> particle_indices;
    glm::vec2 origin{};
    float move_each_step[2]{0, 0};
    float angle_each_step;
};

inline Object::Object(const std::vector<int> &particle_indices,
    const glm::vec2 origin, const glm::vec2 move_each_step, const float rotate_each_step) {
    this->particle_indices = particle_indices;
    this->origin = origin;
    this->move_each_step[0] = move_each_step.x;
    this->move_each_step[1] = move_each_step.y;
    this->angle_each_step = rotate_each_step;
}

inline void Object::move(const float dx, const float dy, Particles &particles) {
    for (const int i : particle_indices) {
        particles.p_x[i] += dx;
        particles.p_y[i] += dy;
    }
    origin.x += dx;
    origin.y += dy;
}

inline void Object::rotate(const float angle, Particles &particles) const {
    const float cos = glm::cos(angle);
    const float sin = glm::sin(angle);

    for (const int i : particle_indices) {
        const float x_offset = particles.p_x[i] - origin.x;
        const float y_offset = particles.p_y[i] - origin.y;

        particles.p_x[i] = origin.x + cos * x_offset - sin * y_offset;
        particles.p_y[i] = origin.y + sin * x_offset + cos * y_offset;
    }
}

inline void Object::update(Particles &particles) {
    const float dx = move_each_step[0];
    const float dy = move_each_step[1];

    if (dx != 0.0f || dy != 0.0f) {
        move(dx, dy, particles);
    }

    if (angle_each_step != 0.0f) {
        rotate(angle_each_step, particles);
    }
}

inline void Object::on_particle_removed(const int removed, const int moved_from) {

    std::erase_if(particle_indices,[&](const int idx) {
        return idx == removed;
    });

    if (moved_from == removed) return;

    for (int &idx : particle_indices) {
        if (idx == moved_from) {
            idx = removed;
            return;
        }
    }
}

#endif //SPH_FLUID_SOLVER_OBJECT_H
