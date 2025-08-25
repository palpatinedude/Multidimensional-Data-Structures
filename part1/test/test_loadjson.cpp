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

    nlohmann::json j;
    in >> j;

    std::cout << "Total trajectories in JSON: " << j.size() << std::endl;

    // Inspect first 3 trajectories
    for (size_t i = 0; i < std::min<size_t>(1, j.size()); ++i) {
        const auto& traj_json = j[i];

        // Print the raw JSON
        std::cout << "\nTrajectory #" << i << " raw JSON:\n" 
                  << traj_json.dump(4) << std::endl;

        // Print all keys
        std::cout << "Keys:\n";
        for (auto it = traj_json.begin(); it != traj_json.end(); ++it) {
            std::cout << " - " << it.key() << std::endl;
        }

        // Try reading ID with different keys
        std::string id1 = traj_json.value("id", "");
        std::string id2 = traj_json.value("trajectory_id", "");
        std::cout << "Read id: '" << id1 << "', Read trajectory_id: '" << id2 << "'\n";

        // Inspect first 3 points
        if (traj_json.contains("points")) {
            const auto& pts = traj_json["points"];
            for (size_t j = 0; j < std::min<size_t>(3, pts.size()); ++j) {
                std::cout << "Point " << j 
                          << ": x=" << pts[j]["x"] 
                          << ", y=" << pts[j]["y"] 
                          << ", t=" << pts[j]["t"] << "\n";
            }
        }
    }

    return 0;
}
