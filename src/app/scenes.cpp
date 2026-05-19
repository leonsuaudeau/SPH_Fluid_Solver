#include "scenes.h"

void Scenes::load_scene_1(FluidSolver &solver) {
    solver.clean_particles();
    solver.add_particle({0,10}, {1,0,0});
    solver.add_particle_grid({20, 2}, {-20 * solver.h, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({20, 2}, {0 * solver.h, -10}, {0.25,0.25,0.25}, true);
}

void Scenes::load_scene_2(FluidSolver &solver) {
    solver.clean_particles();
    solver.add_particle_grid({100, 30}, {-50, -7.3}, {0,0,1});
    solver.add_particle_grid({100, 3}, {-50, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 80}, {-52.7, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 80}, {40, -10}, {0.25,0.25,0.25}, true);
}

void Scenes::load_scene_3(FluidSolver &solver) {
    solver.clean_particles();
    solver.add_particle_grid({40, 50}, {-50, -7.3}, {0,0,1});
    solver.add_particle_grid({100, 3}, {-50, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 80}, {-52.7, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 70}, {-14, -5}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 80}, {40, -10}, {0.25,0.25,0.25}, true);
}

void Scenes::load_scene_4(FluidSolver &solver) {
    solver.clean_particles();
    solver.add_particle_grid({40, 50}, {-50, -7.3}, {0,0,1});
    solver.add_particle_grid({100, 3}, {-50, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 80}, {-52.7, -10}, {0.25,0.25,0.25}, true);
    solver.add_particle_grid({3, 80}, {40, -10}, {0.25,0.25,0.25}, true);
}

void Scenes::add_cubes(FluidSolver &solver) {
    solver.add_particle_grid({10, 10}, {-5, 40}, {1,0,0});
    solver.add_particle_grid({10, 10}, {-5, 60}, {0,1,0});
    solver.add_particle_grid({10, 10}, {-5, 80}, {1,1,0});
}
