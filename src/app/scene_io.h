#ifndef SPH_FLUID_SOLVER_JSON_LOADER_H
#define SPH_FLUID_SOLVER_JSON_LOADER_H
#include <string>
#include "solver.h"

namespace SceneIO {
    void load_from_json(FluidSolver &solver, const std::string &name );
    void save_to_json(FluidSolver &solver, const std::string &name );
}

#endif //SPH_FLUID_SOLVER_JSON_LOADER_H
