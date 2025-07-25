#ifndef RTREE_H
#define RTREE_H

#include <memory>
#include <vector>
#include <string>
#include "RTreeNode.h"
#include "trajectory.h"
#include "bbox3D.h"
#include "../../json.hpp"

class RTree {
private:
    std::shared_ptr<RTreeNode> root;
    int maxEntries;

public:
    RTree(int maxEntries = 8);

    void insert(const Trajectory& traj);
    bool remove(const std::string& trajId);
    bool update(const Trajectory& traj);

    std::vector<Trajectory> rangeQuery(const BoundingBox3D& queryBox) const;
    std::vector<Trajectory> kNearestNeighbors(const Trajectory& query, size_t k) const;
    std::vector<Trajectory> findSimilar(const Trajectory& query, float maxDistance) const;
    void bulkLoad(const std::vector<Trajectory>& trajectories);


    void exportToJSON(const std::string& filename) const;
    void importFromJSON(const std::string& filename);

    size_t getTotalEntries() const;
    int getHeight() const;
    void printStatistics() const;

};

#endif // RTREE_H
