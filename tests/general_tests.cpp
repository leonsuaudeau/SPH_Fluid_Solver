#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "solver.h"
#include "sph_kernel.h"
constexpr float epsilon = 1e-4f;

TEST_CASE("Two particle force symmetry test") {
    constexpr float dt = 0.001f;
    FluidSolver solver(0.9f, 1.1f, 2000, 0.0f, {0,0});
    solver.add_particle({0,0}, {1,0,0});
    solver.add_particle({0.05f, 0}, {0,1,0});
    solver.step(dt);

    REQUIRE(glm::length(solver.particles[0].acc + solver.particles[1].acc) < epsilon);
}

TEST_CASE("No ext. force, density equals rest density") {
    constexpr float dt = 0.001f;
    FluidSolver solver(0.9f, 1.1f, 2000, 0.0f, {0,0});
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(dt);

    REQUIRE(glm::abs(solver.particles[40].density - solver.rho_0) < epsilon);
}

TEST_CASE("Kernel sum with full neighborhood is h^-2") {
    constexpr float dt = 0.001f;
    FluidSolver solver(0.9f, 1.1f, 2000, 0.0f, {0,0});
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(dt);
    float sum = 0;
    for (auto& p_j : solver.get_neighbors(40)) {
        sum += sph::kernels::cubic_spline_2D(solver.particles[40].pos, p_j.pos, solver.h);
    }
    std::cout << sum << std::endl;
    std::cout << 1/(solver.h * solver.h);
    REQUIRE((sum - 1/(solver.h * solver.h)) < epsilon);
}

TEST_CASE("Kernel derivative sum at rest is (0,0)") {
    constexpr float dt = 0.001f;
    FluidSolver solver(0.9f, 1.1f, 2000, 0.0f, {0,0});
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(dt);
    glm::vec2 sum{0,0};
    for (auto& p_j : solver.get_neighbors(40)) {
        sum += sph::kernels::cubic_spline_2D_deriv(solver.particles[40].pos, p_j.pos, solver.h);
    }
    std::cout << sum.x << " " << sum.y << std::endl;
    REQUIRE(sum.x < epsilon);
    REQUIRE(sum.y < epsilon);
}