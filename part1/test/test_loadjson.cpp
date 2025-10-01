#include <iostream>
#include <fstream>
#include "../json.hpp"

using json = nlohmann::json;

int main() {
    std::string filepath = "../preprocessing/trajectories_grouped.json";
    std::ifstream in(filepath);
    if (!in) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return 1;
    }

    json j;
    in >> j;

    std::cout << "Total trajectories in JSON: " << j.size() << "\n\n";

    size_t inspectCount = std::min<size_t>(5, j.size());
    for (size_t idx = 0; idx < inspectCount; ++idx) {
        const auto& traj_json = j[idx];
        if (!traj_json.contains("id")) {
            std::cout << "Trajectory #" << idx << " has no 'id' key\n";
        } else {
            std::cout << "Trajectory #" << idx
                      << " id type: " << traj_json["id"].type_name()
                      << ", value: '" << traj_json["id"] << "'\n";
        }

        if (traj_json.contains("points")) {
            const auto& pts = traj_json["points"];
            size_t pointCount = std::min<size_t>(3, pts.size());
            for (size_t k = 0; k < pointCount; ++k) {
                std::cout << "  Point " << k
                          << ": x=" << pts[k]["x"]
                          << ", y=" << pts[k]["y"]
                          << ", t=" << pts[k]["t"] << "\n";
            }
        }
        std::cout << "\n";
    }

    return 0;
}

