#ifndef SPH_FLUID_SOLVER_STATISTICS_H
#define SPH_FLUID_SOLVER_STATISTICS_H
#include <string>

namespace stats {
    inline std::string relative_path = "../../savestate/statistics/";
    // keep in mind stack size. If there are overflows, consider using pre-initialized std::vector for heap storage
    constexpr int MAX_MEMORY = 100000;
    struct Sequence{
        float density_error[MAX_MEMORY];
        float density_error_no_surface[MAX_MEMORY];
        float time[MAX_MEMORY];
    };

    void save_to_file(const std::string &name, const float* data0, const float* data1, int count);
}

#endif //SPH_FLUID_SOLVER_STATISTICS_H
