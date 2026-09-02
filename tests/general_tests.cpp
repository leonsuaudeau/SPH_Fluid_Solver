#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "solver.h"
#include <iostream>
#include "sph_kernel.h"

constexpr float epsilon = 1e-3f;

TEST_CASE("Two particle force symmetry test") {
    FluidSolver solver(0.0f, 0.9f, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;
    solver.add_particle({0, 0}, {1, 0, 0}, 1.1f);
    solver.add_particle({0.05f, 0}, {0, 1, 0}, 1.1f);
    solver.step(grid, removals);

    const glm::vec2 total_acceleration{
        solver.particles.a_x[0] + solver.particles.a_x[1],
        solver.particles.a_y[0] + solver.particles.a_y[1]
    };
    REQUIRE(glm::length(total_acceleration) < epsilon);
}

TEST_CASE("No ext. force, density equals rest density") {
    FluidSolver solver(0.0f, 0.9f, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;
    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 0}, 1.1f);
    solver.step(grid, removals);

    REQUIRE(glm::abs(solver.particles.rho[40] - solver.particles.rho_0[40]) < epsilon);
}

TEST_CASE("Kernel sum with full neighborhood is h^-2") {
    FluidSolver solver(0.0f, 0.9f, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;
    const sph::kernels::kernel_constants kernel_const(solver.h);
    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 0}, 1.1f);
    solver.step(grid, removals);

    float sum = sph::kernels::cubic_spline_2D(0.0f, kernel_const);
    for (const int j : solver.get_neighbors(40)) {
        const float dx = solver.particles.p_x[40] - solver.particles.p_x[j];
        const float dy = solver.particles.p_y[40] - solver.particles.p_y[j];
        sum += sph::kernels::cubic_spline_2D(dx * dx + dy * dy, kernel_const);
    }
    REQUIRE(glm::abs(sum - 1.0f / (solver.h * solver.h)) < 2.0f * epsilon);
}

TEST_CASE("Kernel derivative sum at rest is (0,0)") {
    FluidSolver solver(0.0f, 0.9f, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;
    const sph::kernels::kernel_constants kernel_const(solver.h);
    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 0}, 1.1f);
    solver.step(grid, removals);

    glm::vec2 sum{0, 0};
    for (const int j : solver.get_neighbors(40)) {
        const float dx = solver.particles.p_x[40] - solver.particles.p_x[j];
        const float dy = solver.particles.p_y[40] - solver.particles.p_y[j];
        sum += sph::kernels::cubic_spline_2D_deriv(dx, dy, dx * dx + dy * dy, kernel_const);
    }
    REQUIRE(glm::length(sum) < epsilon);
}

TEST_CASE("Cross product between d and kernel derivative is scaled identity matrix") {
    FluidSolver solver(0.0f, 0.9, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;
    const sph::kernels::kernel_constants kernel_const(solver.h);

    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 0}, 1.1f);
    solver.step(grid, removals);

    glm::mat2 sum{{0,0},{0,0}};
    for (auto& j : solver.get_neighbors(40)) {
        const float d_p_x = solver.particles.p_x[40] - solver.particles.p_x[j];
        const float d_p_y = solver.particles.p_y[40] - solver.particles.p_y[j];
        const float dot_p_p = d_p_x*d_p_x + d_p_y*d_p_y;

        auto d = glm::vec2(d_p_x, d_p_y);
        sum += glm::outerProduct(d, sph::kernels::cubic_spline_2D_deriv(d_p_x, d_p_y, dot_p_p, kernel_const));
    }
    glm::mat2 error = sum * (solver.h * solver.h) + glm::mat2{{1, 0}, {0, 1}};

    float error_norm = 0;
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 2; i++) {
            error_norm += error[i][j] * error[i][j];
        }
    }
    error_norm = glm::sqrt(error_norm);
    std::cout << "Error: " << error_norm * 100 <<  "%" << std::endl;
    REQUIRE(error_norm < 0.1f);
}

TEST_CASE("Neighbor search full neighborhood middle") {
    FluidSolver solver(0.0f, 0.9, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;

    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 0}, 1.1f);
    solver.step(grid, removals);

    REQUIRE(solver.get_neighbors(40).size() == 12);
}

TEST_CASE("Neighbor search edge") {
    FluidSolver solver(0.0f, 0.9f, 2000.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, 2 * solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;

    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 0}, 1.1f);
    solver.step(grid, removals);
    REQUIRE(solver.get_neighbors(4).size() == 8);
}
