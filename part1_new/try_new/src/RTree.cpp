#include "../include/RTree.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

RTree::RTree(int maxEntries)
    : maxEntries(maxEntries) {
    root = std::make_shared<RTreeNode>(true, maxEntries);
}

void RTree::insert(const Trajectory& traj) {
    std::shared_ptr<RTreeNode> splitNode = root->insertRecursive(traj);
    if (splitNode) {
        auto newRoot = std::make_shared<RTreeNode>(false, maxEntries);
        newRoot->insert(root->getMBR(), root);
        newRoot->insert(splitNode->getMBR(), splitNode);
        root = newRoot;
    }
}

bool RTree::remove(const std::string& trajId) {
    return root->deleteTrajectory(trajId);
}

bool RTree::update(const Trajectory& traj) {
    return root->updateTrajectory(traj);
}

std::vector<Trajectory> RTree::rangeQuery(const BoundingBox3D& queryBox) const {
    std::vector<Trajectory> results;
    root->rangeQuery(queryBox, results);
    return results;
}

std::vector<Trajectory> RTree::kNearestNeighbors(const Trajectory& query, size_t k) const {
    return root->kNearestNeighbors(query, k);
}

std::vector<Trajectory> RTree::findSimilar(const Trajectory& query, float maxDistance) const {
    std::vector<Trajectory> results;
    root->findSimilar(query, maxDistance, results);
    return results;
}

void RTree::exportToJSON(const std::string& filename) const {
    try {
        std::ofstream out(filename);
        if (!out) throw std::runtime_error("Failed to open file for writing");

        json j = root->to_json();
        out << j.dump(2);
        out.close();
    } catch (const std::exception& ex) {
        std::cerr << "Export error: " << ex.what() << std::endl;
    }
}

void RTree::importFromJSON(const std::string& filename) {
    /*
    try {
        std::ifstream in(filename);
        if (!in) throw std::runtime_error("Failed to open file for reading");

        json j;
        in >> j;

        root = std::make_shared<RTreeNode>(true, maxEntries); // Type will be corrected inside from_json
        root->to_json(j);
        in.close();
    } catch (const std::exception& ex) {
        std::cerr << "Import error: " << ex.what() << std::endl;
    }
    */
}

void RTree::bulkLoad(const std::vector<Trajectory>& trajectories) {
    // Sort by space-filling curve (optional, better performance)
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> entries;
    for (const auto& traj : trajectories) {
        entries.emplace_back(traj.computeBoundingBox(), std::make_shared<Trajectory>(traj));
    }

    // Sort entries by minX for spatial locality (better than random)
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.first.getMinX() < b.first.getMinX();
    });

    // Bottom-up build
    std::vector<std::shared_ptr<RTreeNode>> leaves;
    for (size_t i = 0; i < entries.size(); i += maxEntries) {
        auto node = std::make_shared<RTreeNode>(true, maxEntries);
        size_t end = std::min(i + maxEntries, entries.size());
        for (size_t j = i; j < end; ++j) {
            node->insertLeafEntry(entries[j].first, entries[j].second);
        }
        leaves.push_back(node);
    }

    // Build upper levels
    while (leaves.size() > 1) {
        std::vector<std::shared_ptr<RTreeNode>> parents;
        for (size_t i = 0; i < leaves.size(); i += maxEntries) {
            auto parent = std::make_shared<RTreeNode>(false, maxEntries);
            size_t end = std::min(i + maxEntries, leaves.size());
            for (size_t j = i; j < end; ++j) {
                parent->insert(leaves[j]->getMBR(), leaves[j]);
            }
            parents.push_back(parent);
        }
        leaves = parents;
    }

    root = leaves.front();
}

size_t RTree::getTotalEntries() const {
    if (!root) return 0;

    std::queue<std::shared_ptr<RTreeNode>> q;
    q.push(root);
    size_t count = 0;

    while (!q.empty()) {
        auto node = q.front(); q.pop();
        if (node->isLeafNode()) {
            count += node->getLeafEntries().size();
        } else {
            for (const auto& [_, child] : node->getChildEntries()) {
                q.push(child);
            }
        }
    }

    return count;
}
int RTree::getHeight() const {
    auto node = root;
    int height = 0;

    while (node && !node->isLeafNode()) {
        const auto& children = node->getChildEntries();
        if (!children.empty()) {
            node = children.front().second;
        } else {
            break; // Safety check in case of malformed tree
        }
        ++height;
    }

    return node ? height + 1 : 0; // Include leaf level if node exists
}


void RTree::printStatistics() const {
    std::cout << "========= RTree Statistics =========" << std::endl;
    std::cout << "Total entries: " << getTotalEntries() << std::endl;
    std::cout << "Tree height: " << getHeight() << std::endl;
    std::cout << "Max entries per node: " << maxEntries << std::endl;
}
