#include "scene_io.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

std::string relative_path = "../../savestate/";

void SceneIO::load_from_json(FluidSolver &solver, const std::string &name, const std::string &root ) {
    std::ifstream json_in(relative_path + root + name + ".json");
    nlohmann::json json;
    if (!json_in) {
        throw std::runtime_error("Failed to open json file");
    }
    json_in >> json;

    solver.dt = json["dt"];
    solver.h = json["h"];
    solver.rho_0 = json["rho_0"];
    solver.k = json["k"];
    solver.nu = json["nu"];
    solver.cfl = json["cfl"];
    solver.g = {json["g"][0], json["g"][1]};
    std::string data_file = json["data_file"];

    std::ifstream bin_in(data_file);
    if (!bin_in) {
        throw std::runtime_error("Failed to open bin file");
    }
    bin_in.seekg(0, std::ios::end);
    size_t size = bin_in.tellg();
    bin_in.seekg(0, std::ios::beg);

    size_t count = size / sizeof(Particle2D);
    solver.particles = std::vector<Particle2D>(count);
    bin_in.read(
        reinterpret_cast<char*>(solver.particles.data()),
        size
    );
}

void SceneIO::save_to_json(FluidSolver &solver, const std::string &name, const std::string &root ) {
    nlohmann::json json;
    json["dt"] = solver.dt;
    json["h"] = solver.h;
    json["rho_0"] = solver.rho_0;
    json["k"] = solver.k;
    json["nu"] = solver.nu;
    json["cfl"] = solver.cfl;
    json["g"] = {solver.g.x, solver.g.y};
    json["data_file"] = relative_path + root + name + ".bin";

    std::ofstream json_out(relative_path +  root + name + ".json");
    if (!json_out) {
        throw std::runtime_error("Failed to open json file");
    }
    std::ofstream bin_out(relative_path + root + name +".bin", std::ios::binary);
    if (!bin_out) {
        throw std::runtime_error("Failed to open bin file");
    }

    json_out << json.dump(4);
    bin_out.write(
        reinterpret_cast<const char*>(solver.particles.data()),
        solver.get_num_particles() * sizeof(Particle2D));
}

std::vector<std::string> SceneIO::get_scene_entries(const std::string &root) {
    std::vector<std::string> scene_entries{};
    for (const auto &entry : std::filesystem::directory_iterator(relative_path + root)) {
        if (entry.path().extension() == ".json") {
            scene_entries.push_back(entry.path().filename().replace_extension(""));
        }
    }
    return scene_entries;
}

void SceneIO::remove_scene_entry(const std::string &name, const std::string &root) {
    for (const auto &entry : std::filesystem::directory_iterator(relative_path + root)) {
        std::cout << relative_path + root <<std::endl;
        if (entry.path().filename().replace_extension("") == name) {
            remove(entry);
        }
    }
}
