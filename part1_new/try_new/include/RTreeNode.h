
/*
#ifndef RTREENODE_H
#define RTREENODE_H

#include <vector>
#include <memory>
#include <utility>
#include "bbox3D.h"
#include "trajectory.h"
#include "../../json.hpp"

using json = nlohmann::json;

class RTreeNode {
private:
    int maxEntries;
    bool isLeaf;
    BoundingBox3D nodeMBR;

    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> leafEntries;
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>> childEntries;

public:
    RTreeNode(bool isLeaf, int maxEntries);

    void insertRecursive(const Trajectory& traj);
    void insert(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child);

    void rangeQuery(const BoundingBox3D& queryBox, std::vector<Trajectory>& results) const;

    bool isLeafNode() const;
    BoundingBox3D getMBR() const;
    bool needsSplit() const;

    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitLeaf();
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitInternal();

    void updateMBR();
    json to_json() const;
    bool deleteTrajectory(const std::string& trajId);  // returns true if deleted
    bool updateTrajectory(const Trajectory& traj);  // updates by id
    void findSimilar(const Trajectory& query, float maxDistance, std::vector<Trajectory>& results) const;
    std::vector<Trajectory> kNearestNeighbors(const Trajectory& query, size_t k) const;

    float enlargement(const BoundingBox3D& current, const BoundingBox3D& toInclude) const;
    int chooseSubtree(const BoundingBox3D& box) const;

private:
    //float enlargement(const BoundingBox3D& current, const BoundingBox3D& toInclude) const;
    //int chooseSubtree(const BoundingBox3D& box) const;
};

#endif // RTREENODE_H

*/
#ifndef RTREENODE_H
#define RTREENODE_H

#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <queue>
#include "bbox3D.h"
#include "trajectory.h"
#include "../../json.hpp"

using json = nlohmann::json;

class RTreeNode : public std::enable_shared_from_this<RTreeNode> {
private:
    int maxEntries;
    bool isLeaf;
    mutable bool mbrDirty = true;
    mutable BoundingBox3D nodeMBR;
    std::weak_ptr<RTreeNode> parent;

    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> leafEntries;
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>> childEntries;

    void condenseTree();

public:
    RTreeNode(bool isLeaf, int maxEntries);

    bool isLeafNode() const;
    BoundingBox3D getMBR() const;
    void markDirty();
    bool needsSplit() const;

    std::shared_ptr<RTreeNode> insertRecursive(const Trajectory& traj);
    void insert(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child);
    void insertLeafEntry(const BoundingBox3D& box, std::shared_ptr<Trajectory> traj);

    
    void rangeQuery(const BoundingBox3D& queryBox, std::vector<Trajectory>& results) const;
    void updateMBR() const;

    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitLeaf();
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitInternal();

    const std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& getLeafEntries() const;
    const std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>& getChildEntries() const;


    bool deleteTrajectory(const std::string& trajId);
    bool updateTrajectory(const Trajectory& traj);
    void findSimilar(const Trajectory& query, float maxDistance, std::vector<Trajectory>& results) const;
    std::vector<Trajectory> kNearestNeighbors(const Trajectory& query, size_t k) const;

    float enlargement(const BoundingBox3D& current, const BoundingBox3D& toInclude) const;
    int chooseSubtree(const BoundingBox3D& box) const;

    json to_json() const;
};

#endif // RTREENODE_H

