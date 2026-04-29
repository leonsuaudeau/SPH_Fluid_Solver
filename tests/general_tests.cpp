#include <catch2/catch_test_macros.hpp>
#include "solver.h"
constexpr float epsilon = 0.001f;

TEST_CASE("Two particle force symmetry test") {
    std::vector<Particle2D> particles;
    particles.push_back(Particle2D(glm::vec2(0, 0), glm::vec2(0), glm::vec2(0), 1, 0, 1));
    particles.push_back(Particle2D(glm::vec2(0.05f, 0), glm::vec2(0), glm::vec2(0), 1, 0, 1));

    fluid_solver_iteration(particles, 0.001f, 1.0f);
    REQUIRE(glm::length(particles[0].acc + particles[1].acc) < epsilon);
}

TEST_CASE("No ext. force, density equals rest density") {

}