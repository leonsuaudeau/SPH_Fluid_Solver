#ifndef SPH_FLUID_SOLVER_JSON_LOADER_H
#define SPH_FLUID_SOLVER_JSON_LOADER_H
#include <string>
#include "solver.h"

namespace SceneIO {
    inline std::string relative_path = "../../savestate/";
    void load_from_json(FluidSolver &solver, const std::string &name, const std::string &root = "");
    void save_to_json(FluidSolver &solver, const std::string &name, const std::string &root = "" );
    std::vector<std::string> get_scene_entries(const std::string &root = "");
    void remove_scene_entry(const std::string &name, const std::string &root = "");
}

#endif //SPH_FLUID_SOLVER_JSON_LOADER_H
