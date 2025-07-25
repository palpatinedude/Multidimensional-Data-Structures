#include <fstream>
#include <iostream>
#include <vector>
#include "../include/trajectory.h"
#include "../include/RTree.h"


using json = nlohmann::json;

int main() {
    std::ifstream in("/home/marianthi/Desktop/ceid/semester_9/multidimensional/data/trajectories_grouped.json");
    if (!in) {
    perror("Error opening JSON file");
    return 1;
}


    json j;
    in >> j;

    std::vector<Trajectory> trajectories;

    for (const auto& traj_json : j) {
        std::string traj_id = traj_json["id"];
        Trajectory traj(traj_id);

        for (const auto& pt : traj_json["points"]) {
            float x = pt["x"];
            float y = pt["y"];
            float t = pt["t"];
            traj.points.emplace_back(x, y, t);
        }

        trajectories.push_back(traj);
    }

    RTree rtree(8); 

    // Bulk load all trajectories
    rtree.bulkLoad(trajectories);

    std::cout << "RTree loaded with " << rtree.getTotalEntries() << " trajectories." << std::endl;
    rtree.printStatistics();

    return 0;
}
