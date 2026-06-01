#ifndef SPH_FLUID_SOLVER_STATISTICS_H
#define SPH_FLUID_SOLVER_STATISTICS_H
#include <string>

namespace stats {
    constexpr int MAX_MEMORY = 1000000;
    struct Sequence{
        float rho_avg[MAX_MEMORY];
        float time[MAX_MEMORY];
    };

    void load_from_file(const std::string &name);
    void save_to_file(const std::string &name);
}

#endif //SPH_FLUID_SOLVER_STATISTICS_H
