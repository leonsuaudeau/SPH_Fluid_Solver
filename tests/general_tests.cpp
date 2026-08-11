#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "solver.h"

TEST_CASE("Grid resize keeps storage in sync with dimensions") {
    FluidSolver solver(0.0f, 1.0f, 1.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(2, 2, {0, 0}, solver.h, solver.particles);

    solver.add_particle({3.5f, 3.5f}, {1, 0, 0}, 1.1f);
    grid.resize(4, 4);
    grid.populate_cells();

    const int cell = grid.get_cell_index(3.5f, 3.5f);
    REQUIRE(cell == 15);
    REQUIRE(grid.counts.size() == 16);
    REQUIRE(grid.particle_indices.size() == 16 * MAX_PARTICLES_PER_CELL);
    REQUIRE(grid.counts[cell] == 1);
    REQUIRE(grid.particle_indices[cell * MAX_PARTICLES_PER_CELL] == 0);
}

TEST_CASE("Grid cell lookup floors coordinates at the lower boundary") {
    FluidSolver solver(0.0f, 1.0f, 1.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(2, 2, {0, 0}, solver.h, solver.particles);

    REQUIRE(grid.get_cell_index(-0.1f, 0.5f) == -1);
    REQUIRE(grid.get_cell_index(0.5f, -0.1f) == -1);
    REQUIRE(grid.get_cell_index(0.5f, 0.5f) == 0);
}

TEST_CASE("Out-of-domain particle removal handles swap-remove indices") {
    FluidSolver solver(0.0f, 1.0f, 1.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(10, 10, {0, 0}, solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;

    solver.add_particle({1, 1}, {1, 0, 0}, 1.1f);
    solver.add_particle({20, 20}, {1, 0, 0}, 1.1f);
    solver.add_particle({2, 2}, {1, 0, 0}, 1.1f);
    solver.add_particle({3, 3}, {1, 0, 0}, 1.1f);
    solver.add_particle({21, 21}, {1, 0, 0},1.1f);

    solver.step(grid, removals);

    REQUIRE(solver.get_num_particles() == 3);
    for (int i = 0; i < solver.get_num_particles(); i++) {
        REQUIRE(grid.get_cell_index(solver.particles.p_x[i], solver.particles.p_y[i]) != -1);
    }
}

TEST_CASE("Neighbor search finds a full center neighborhood") {
    FluidSolver solver(0.0f, 1.0f, 1.0f, 0.0f, {0, 0}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Grid grid(32, 32, {-8, -8}, solver.h, solver.particles);
    std::vector<ParticleRemoval> removals;

    solver.add_particle_grid({9, 9}, {0, 0}, {0, 0, 1}, 1.1f);
    solver.step(grid, removals);

    REQUIRE(solver.get_neighbors(40).size() == 12);
}

// Legacy tests kept for reference. They target an older solver/data-structure API
// and are intentionally not compiled with the current architecture.
#if 0
#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "solver.h"
#include "sph_kernel.h"
constexpr float epsilon = 1e-3f;

TEST_CASE("Two particle force symmetry test") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 2000, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    solver.add_particle({0,0}, {1,0,0});
    solver.add_particle({0.05f, 0}, {0,1,0});
    solver.step(grid);

    REQUIRE(glm::length(solver.particles[0].acc + solver.particles[1].acc) < epsilon);
}

TEST_CASE("No ext. force, density equals rest density") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 2000, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(grid);

    REQUIRE(glm::abs(solver.particles[40].density - solver.rho_0) < epsilon);
}

TEST_CASE("Kernel sum with full neighborhood is h^-2") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 2000, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(grid);
    float sum = 0;
    for (auto& p_j : solver.get_neighbors(40)) {
        sum += sph::kernels::cubic_spline_2D(solver.particles[40].pos, p_j.pos, solver.h);
    }
    std::cout << sum << std::endl;
    std::cout << 1/(solver.h * solver.h);
    REQUIRE((sum - 1/(solver.h * solver.h)) < epsilon);
}

TEST_CASE("Kernel derivative sum at rest is (0,0)") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 2000, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(grid);
    glm::vec2 sum{0,0};
    for (auto& p_j : solver.get_neighbors(40)) {
        sum += sph::kernels::cubic_spline_2D_deriv(solver.particles[40].pos, p_j.pos, solver.h);
    }
    std::cout << sum.x << " " << sum.y << std::endl;
    REQUIRE(sum.x < epsilon);
    REQUIRE(sum.y < epsilon);
}

TEST_CASE("Cross product between d and kernel derivative is scaled identity matrix") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 0, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    glm::ivec2 particle_count{9, 9};
    glm::vec2 grid_origin{0, 0};
    solver.add_particle_grid(particle_count, grid_origin, {0,0,0});
    solver.step(grid);
    glm::mat2 sum{{0,0},{0,0}};
    for (auto& p_j : solver.get_neighbors(40)) {
        Particle2D p_i = solver.particles[40];
        glm::vec2 d = p_i.pos - p_j.pos;
        sum += glm::outerProduct(d, sph::kernels::cubic_spline_2D_deriv(p_i.pos, p_j.pos, solver.h));
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

TEST_CASE("Neighbor search full neighborhood") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 2000, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    solver.add_particle_grid({9,9}, {0,0}, {0,0,0});
    solver.step(grid);
    REQUIRE(solver.neighbor_indices[40].size() == 13);
}

TEST_CASE("Neighbor search edge") {
    FluidSolver solver(0.001f, 0.9f, 1.1f, 2000, 0.0f, {0,0});
    DataStructure::Grid grid(512, 512, {-256, -256}, solver);
    solver.add_particle_grid({9,9}, {0,0}, {0,0,0});
    solver.step(grid);
    REQUIRE(solver.neighbor_indices[4].size() == 9);
}
#endif
