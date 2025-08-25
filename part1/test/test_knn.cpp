#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include "../api/include/trajectory.h"
#include "../api/include/RTree.h"
#include "../api/include/bbox3D.h"
#include "../api/include/point3D.h"

// Helper to create a trajectory
Trajectory createTrajectory(const std::string& id, const std::vector<std::tuple<float,float,std::string>>& points) {
    std::vector<Point3D> pts;
    for (auto& [x, y, timestamp] : points) {
        pts.emplace_back(x, y, timestamp);
    }
    return Trajectory(pts, id);
}

// Linear scan helper
std::vector<std::pair<float, Trajectory*>> linearKNN(const Trajectory& query, std::vector<Trajectory>& dataset, size_t k) {
    std::vector<std::pair<float, Trajectory*>> distances;
    for (auto& traj : dataset) {
        float dist = query.similarityTo(traj);
        distances.push_back({dist, &traj});
    }
    std::sort(distances.begin(), distances.end(), [](auto &a, auto &b){ return a.first < b.first; });
    if (distances.size() > k) distances.resize(k);
    return distances;
}

int main() {
    // ---------------- Create dataset ----------------
    std::vector<Trajectory> trajectories;
    trajectories.push_back(createTrajectory("T1", {{0,0,"2025-08-24T10:00:00Z"}, {1,1,"2025-08-24T10:05:00Z"}}));
    trajectories.push_back(createTrajectory("T2", {{2,2,"2025-08-24T10:02:00Z"}, {3,3,"2025-08-24T10:07:00Z"}}));
    trajectories.push_back(createTrajectory("T3", {{1,1,"2025-08-24T10:01:00Z"}, {2,2,"2025-08-24T10:06:00Z"}}));
    trajectories.push_back(createTrajectory("T4", {{5,5,"2025-08-24T10:03:00Z"}, {6,6,"2025-08-24T10:08:00Z"}}));

    // Copy for linear scan comparison
    std::vector<Trajectory> trajectoriesCopy = trajectories;

    // ---------------- Build RTree ----------------
    RTree rtree(2);
    rtree.bulkLoad(trajectories);

    // ---------------- Query trajectory ----------------
    Trajectory query = createTrajectory("Q", {{0,0,"2025-08-24T10:01:00Z"}, {1,0.5,"2025-08-24T10:04:00Z"}});

    // ---------------- Linear scan ----------------
    size_t k = 2;
    auto linearResults = linearKNN(query, trajectoriesCopy, k);

    std::cout << "Linear scan nearest neighbors:\n";
    for (size_t i=0; i<linearResults.size(); ++i) {
        std::cout << i+1 << ": " << linearResults[i].second->getId()
                  << " dist=" << linearResults[i].first << "\n";
    }

    // ---------------- RTree KNN ----------------
    auto rtreeResults = rtree.kNearestNeighbors(query, k);

    std::cout << "\nRTree KNN results (k=" << k << "):\n";
    for (size_t i=0; i<rtreeResults.size(); ++i) {
        std::cout << i+1 << ": " << rtreeResults[i].getId()
                  << " dist=" << query.similarityTo(rtreeResults[i]) << "\n";
    }

    // ---------------- Compare top-k ----------------
    for (size_t i=0; i<k; ++i) {
        assert(rtreeResults[i].getId() == linearResults[i].second->getId() &&
               "RTree KNN does not match linear scan!");
    }

    std::cout << "\nKNN test passed! RTree matches linear scan.\n";
    return 0;
}
