#include <fstream>
#include "statistics.h"

void stats::load_from_file(const std::string &name) {

}

void stats::save_to_file(const std::string &name, const float *data0, const float *data1, const int count) {
    std::ofstream bin_out(relative_path + name +".bin", std::ios::binary);
    if (!bin_out) {
        throw std::runtime_error("Failed to open bin file");
    }

    bin_out.write(reinterpret_cast<const char*>(data0), count * sizeof(float));
    bin_out.write(reinterpret_cast<const char*>(data1), count * sizeof(float));

    if (!bin_out) {
        throw std::runtime_error("Failed to write to bin file");
    }
}