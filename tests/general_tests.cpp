#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "solver.h"
#include "sph_math.h"
constexpr float epsilon = 1e-4f;

TEST_CASE("Two particle force symmetry test") {
    std::vector<Particle2D> particles;
    particles.push_back(Particle2D(glm::vec2(0, 0), glm::vec2(0), glm::vec2(0), 1, 0, 1));
    particles.push_back(Particle2D(glm::vec2(0.05f, 0), glm::vec2(0), glm::vec2(0), 1, 0, 1));

    fluid_solver_iteration(particles, 0.001f, 1.0f, 1.0f, 10);
    REQUIRE(glm::length(particles[0].acc + particles[1].acc) < epsilon);
}

TEST_CASE("No ext. force, density equals rest density") {
    std::vector<Particle2D> particles;
    float dx = 0.1f;
    float h = 2.0f * dx;
    float rho_0 = 1.0f;
    float m_i = dx * dx * rho_0;
    glm::ivec2 particle_count = glm::ivec2(9, 9);
    glm::vec2 grid_origin = glm::ivec2(-1, -1);

    for (int y = 0; y < particle_count.y; y++ ) {
        for (int x = 0; x < particle_count.x; x++) {
            glm::vec2 pos = grid_origin + glm::vec2(x, y) * dx;
            particles.emplace_back(Particle2D(pos, glm::vec2(0), glm::vec2(0), m_i, 0, rho_0));
        }
    }
    fluid_solver_iteration(particles, 0.001f, h, rho_0, 10);
    auto neighbors = get_neighbors(particles, h);
    std::cout << particles[40].density << std::endl;
    std::cout << neighbors[40].size() << std::endl;

    REQUIRE(glm::abs(particles[40].density - rho_0) < epsilon);
}