/*
#include "../include/RTreeNode.h"
#include "../include/bbox3D.h"
#include "../include/trajectory.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <queue>

struct HeapEntry {
    float distance;
    std::shared_ptr<RTreeNode> node;
    bool operator<(const HeapEntry& other) const {
        return distance > other.distance;  // min-heap
    }
};

struct ResultEntry {
    float distance;
    Trajectory traj;
    bool operator<(const ResultEntry& other) const {
        return distance < other.distance; // max-heap for results (to pop worst)
    }
};


RTreeNode::RTreeNode(bool isLeaf, int maxEntries)
    : isLeaf(isLeaf), maxEntries(maxEntries) {}

bool RTreeNode::isLeafNode() const {
    return isLeaf;
}

BoundingBox3D RTreeNode::getMBR() const {
    return nodeMBR;
}

bool RTreeNode::needsSplit() const {
    return isLeaf ? leafEntries.size() > maxEntries : childEntries.size() > maxEntries;
}

float RTreeNode::enlargement(const BoundingBox3D& current, const BoundingBox3D& toInclude) const {
    BoundingBox3D combined = current;
    combined.expandToInclude(toInclude);
    return combined.volume() - current.volume();
}

int RTreeNode::chooseSubtree(const BoundingBox3D& box) const {
    float minEnlargement = std::numeric_limits<float>::max();
    int bestIndex = -1;

    for (size_t i = 0; i < childEntries.size(); ++i) {
        float enlarge = enlargement(childEntries[i].first, box);
        if (enlarge < minEnlargement) {
            minEnlargement = enlarge;
            bestIndex = i;
        }
    }

    return bestIndex;
}

void RTreeNode::insertRecursive(const Trajectory& traj) {
    BoundingBox3D box = traj.computeBoundingBox();

    if (isLeaf) {
        leafEntries.emplace_back(box, std::make_shared<Trajectory>(traj));
        nodeMBR.expandToInclude(box);
    } else {
        int idx = chooseSubtree(box);
        auto& [childBox, child] = childEntries[idx];
        child->insertRecursive(traj);

        childBox.expandToInclude(box);
        nodeMBR.expandToInclude(box);
    }
}

void RTreeNode::insert(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child) {
    childEntries.emplace_back(box, child);
    nodeMBR.expandToInclude(box);
}

void RTreeNode::rangeQuery(const BoundingBox3D& queryBox, std::vector<Trajectory>& results) const {
    if (!nodeMBR.intersects(queryBox)) return;

    if (isLeaf) {
        for (const auto& [box, traj] : leafEntries) {
            if (queryBox.intersects(box)) {
                results.push_back(*traj);
            }
        }
    } else {
        for (const auto& [box, child] : childEntries) {
            if (queryBox.intersects(box)) {
                child->rangeQuery(queryBox, results);
            }
        }
    }
}

void RTreeNode::updateMBR() {
    nodeMBR = BoundingBox3D();
    if (isLeaf) {
        for (const auto& [box, _] : leafEntries)
            nodeMBR.expandToInclude(box);
    } else {
        for (const auto& [box, _] : childEntries)
            nodeMBR.expandToInclude(box);
    }
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitLeaf() {
    std::cout << "Splitting leaf node...\n";
    size_t i1 = 0, i2 = 1;
    float maxDist = 0;

    for (size_t i = 0; i < leafEntries.size(); ++i) {
        for (size_t j = i + 1; j < leafEntries.size(); ++j) {
            float dist = leafEntries[i].first.spatialDistanceSquared(leafEntries[j].first);
            if (dist > maxDist) {
                maxDist = dist;
                i1 = i;
                i2 = j;
            }
        }
    }

    auto left = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    left->insertRecursive(*leafEntries[i1].second);
    right->insertRecursive(*leafEntries[i2].second);

    for (size_t i = 0; i < leafEntries.size(); ++i) {
        if (i == i1 || i == i2) continue;
        const auto& traj = leafEntries[i].second;
        auto box = traj->computeBoundingBox();
        if (enlargement(left->getMBR(), box) < enlargement(right->getMBR(), box))
            left->insertRecursive(*traj);
        else
            right->insertRecursive(*traj);
    }

    left->updateMBR();
    right->updateMBR();
    return {left, right};
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitInternal() {
    std::cout << "Splitting internal node...\n";
    size_t i1 = 0, i2 = 1;
    float maxDist = 0;

    for (size_t i = 0; i < childEntries.size(); ++i) {
        for (size_t j = i + 1; j < childEntries.size(); ++j) {
            float dist = childEntries[i].first.spatialDistanceSquared(childEntries[j].first);
            if (dist > maxDist) {
                maxDist = dist;
                i1 = i;
                i2 = j;
            }
        }
    }

    auto left = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    left->insert(childEntries[i1].first, childEntries[i1].second);
    right->insert(childEntries[i2].first, childEntries[i2].second);

    for (size_t i = 0; i < childEntries.size(); ++i) {
        if (i == i1 || i == i2) continue;
        const auto& entry = childEntries[i];
        if (enlargement(left->getMBR(), entry.first) < enlargement(right->getMBR(), entry.first))
            left->insert(entry.first, entry.second);
        else
            right->insert(entry.first, entry.second);
    }

    left->updateMBR();
    right->updateMBR();
    return {left, right};
}

json RTreeNode::to_json() const {
    json j;
    j["type"] = isLeafNode() ? "leaf" : "internal";
    j["mbr"] = nodeMBR.to_json();

    if (isLeafNode()) {
        for (const auto& [box, traj] : leafEntries) {
            j["trajectories"].push_back(traj->to_json());
        }
    } else {
        for (const auto& [box, child] : childEntries) {
            j["children"].push_back(child->to_json());
        }
    }

    return j;
}


bool RTreeNode::deleteTrajectory(const std::string& trajId) {
    if (isLeaf) {
        for (auto it = leafEntries.begin(); it != leafEntries.end(); ++it) {
            if (it->second->id == trajId) {
                leafEntries.erase(it);
                updateMBR();
                return true;
            }
        }
        return false;
    } else {
        for (auto& [box, child] : childEntries) {
            if (child->deleteTrajectory(trajId)) {
                updateMBR();
                return true;
            }
        }
        return false;
    }
}

bool RTreeNode::updateTrajectory(const Trajectory& traj) {
    if (!deleteTrajectory(traj.id))
        return false; // Not found to update

    insertRecursive(traj);  // Reinsert updated trajectory
    return true;
}

void RTreeNode::findSimilar(const Trajectory& query, float maxDistance, std::vector<Trajectory>& results) const {
    if (!nodeMBR.intersects(query.computeBoundingBox()))
        return;

    if (isLeaf) {
        for (const auto& [box, traj] : leafEntries) {
            if (traj->similarityTo(query) <= maxDistance) {
                results.push_back(*traj);
            }
        }
    } else {
        for (const auto& [box, child] : childEntries) {
            if (box.intersects(query.computeBoundingBox())) {
                child->findSimilar(query, maxDistance, results);
            }
        }
    }
}

std::vector<Trajectory> RTreeNode::kNearestNeighbors(const Trajectory& query, size_t k) const {
    std::priority_queue<HeapEntry> toVisit;
    std::priority_queue<ResultEntry> bestResults;

    toVisit.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    while (!toVisit.empty()) {
        auto current = toVisit.top();
        toVisit.pop();

        if (isLeafNode()) {
            for (const auto& [box, trajPtr] : leafEntries) {
                float dist = trajPtr->similarityTo(query);
                if (bestResults.size() < k) {
                    bestResults.push({dist, *trajPtr});
                } else if (dist < bestResults.top().distance) {
                    bestResults.pop();
                    bestResults.push({dist, *trajPtr});
                }
            }
        } else {
            for (const auto& [box, child] : childEntries) {
                float dist = box.distanceTo(query.computeBoundingBox());
                if (bestResults.size() < k || dist < bestResults.top().distance) {
                    toVisit.push({dist, child});
                }
            }
        }
    }

    std::vector<Trajectory> results;
    while (!bestResults.empty()) {
        results.push_back(bestResults.top().traj);
        bestResults.pop();
    }
    std::reverse(results.begin(), results.end());
    return results;
}
*/
#include "../include/RTreeNode.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <queue>
#include <cmath>

// Helpers
struct HeapEntry {
    float distance;
    std::shared_ptr<RTreeNode> node;
    bool operator<(const HeapEntry& other) const {
        return distance > other.distance;
    }
};

struct ResultEntry {
    float distance;
    Trajectory traj;
    bool operator<(const ResultEntry& other) const {
        return distance < other.distance;
    }
};

RTreeNode::RTreeNode(bool isLeaf, int maxEntries)
    : isLeaf(isLeaf), maxEntries(maxEntries) {}

bool RTreeNode::isLeafNode() const {
    return isLeaf;
}

BoundingBox3D RTreeNode::getMBR() const {
    if (mbrDirty) updateMBR();
    return nodeMBR;
}

void RTreeNode::markDirty() {
    mbrDirty = true;
    if (auto p = parent.lock()) p->markDirty();
}

bool RTreeNode::needsSplit() const {
    return isLeaf ? leafEntries.size() > maxEntries : childEntries.size() > maxEntries;
}

float RTreeNode::enlargement(const BoundingBox3D& current, const BoundingBox3D& toInclude) const {
    BoundingBox3D combined = current;
    combined.expandToInclude(toInclude);
    return combined.volume() - current.volume();
}

int RTreeNode::chooseSubtree(const BoundingBox3D& box) const {
    float minEnlargement = std::numeric_limits<float>::max();
    int bestIndex = -1;
    for (size_t i = 0; i < childEntries.size(); ++i) {
        float enlarge = enlargement(childEntries[i].first, box);
        if (enlarge < minEnlargement) {
            minEnlargement = enlarge;
            bestIndex = static_cast<int>(i);
        }
    }
    return bestIndex;
}

// [CHANGED] insertRecursive now returns split node
std::shared_ptr<RTreeNode> RTreeNode::insertRecursive(const Trajectory& traj) {
    BoundingBox3D box = traj.computeBoundingBox();

    if (isLeaf) {
        leafEntries.emplace_back(box, std::make_shared<Trajectory>(traj));
        markDirty();

        if (needsSplit()) {
            auto [left, right] = splitLeaf();
            if (auto p = parent.lock()) {
                p->childEntries.erase(std::remove_if(p->childEntries.begin(), p->childEntries.end(),
                    [&](const auto& pair) { return pair.second.get() == this; }), p->childEntries.end());

                p->insert(left->getMBR(), left);
                p->insert(right->getMBR(), right);
            }
            return right; // indicate split
        }
    } else {
        int idx = chooseSubtree(box);
        auto& [childBox, child] = childEntries[idx];
        std::shared_ptr<RTreeNode> splitNode = child->insertRecursive(traj);

        if (splitNode) {
            insert(splitNode->getMBR(), splitNode);
            if (needsSplit()) {
                auto [left, right] = splitInternal();
                if (auto p = parent.lock()) {
                    p->childEntries.erase(std::remove_if(p->childEntries.begin(), p->childEntries.end(),
                        [&](const auto& pair) { return pair.second.get() == this; }), p->childEntries.end());
                    p->insert(left->getMBR(), left);
                    p->insert(right->getMBR(), right);
                }
                return right;
            }
        }
        childBox.expandToInclude(box);
        markDirty();
    }

    return nullptr;
}

void RTreeNode::insert(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child) {
    childEntries.emplace_back(box, child);
    child->parent = shared_from_this();
    markDirty();
}

void RTreeNode::rangeQuery(const BoundingBox3D& queryBox, std::vector<Trajectory>& results) const {
    if (!getMBR().intersects(queryBox)) return;
    if (isLeaf) {
        for (const auto& [box, traj] : leafEntries) {
            if (queryBox.intersects(box)) results.push_back(*traj);
        }
    } else {
        for (const auto& [box, child] : childEntries) {
            if (queryBox.intersects(box)) child->rangeQuery(queryBox, results);
        }
    }
}

void RTreeNode::updateMBR() const {
    nodeMBR = BoundingBox3D();
    if (isLeaf) {
        for (const auto& [box, _] : leafEntries)
            nodeMBR.expandToInclude(box);
    } else {
        for (const auto& [box, _] : childEntries)
            nodeMBR.expandToInclude(box);
    }
    mbrDirty = false;
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitLeaf() {
    std::sort(leafEntries.begin(), leafEntries.end(), [](const auto& a, const auto& b) {
        return a.first.getMinX() < b.first.getMinX();
    });

    size_t mid = leafEntries.size() / 2;
    auto left = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    left->leafEntries.assign(leafEntries.begin(), leafEntries.begin() + mid);
    right->leafEntries.assign(leafEntries.begin() + mid, leafEntries.end());

    left->markDirty();
    right->markDirty();

    return {left, right};
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitInternal() {
    std::sort(childEntries.begin(), childEntries.end(), [](const auto& a, const auto& b) {
        return a.first.getMinX() < b.first.getMinX();
    });

    size_t mid = childEntries.size() / 2;
    auto left = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    left->childEntries.assign(childEntries.begin(), childEntries.begin() + mid);
    right->childEntries.assign(childEntries.begin() + mid, childEntries.end());

    for (auto& [_, child] : left->childEntries)
        child->parent = left;
    for (auto& [_, child] : right->childEntries)
        child->parent = right;

    left->markDirty();
    right->markDirty();

    return {left, right};
}

bool RTreeNode::deleteTrajectory(const std::string& trajId) {
    if (isLeaf) {
        auto it = std::remove_if(leafEntries.begin(), leafEntries.end(), [&](const auto& pair) {
            return pair.second->id == trajId;
        });
        if (it != leafEntries.end()) {
            leafEntries.erase(it, leafEntries.end());
            markDirty();
            condenseTree();
            return true;
        }
        return false;
    } else {
        for (auto& [_, child] : childEntries) {
            if (child->deleteTrajectory(trajId)) {
                markDirty();
                return true;
            }
        }
        return false;
    }
}

void RTreeNode::condenseTree() {
    if (auto p = parent.lock()) {
        if ((isLeaf && leafEntries.empty()) || (!isLeaf && childEntries.empty())) {
            auto& siblings = p->childEntries;
            siblings.erase(std::remove_if(siblings.begin(), siblings.end(), [&](const auto& pair) {
                return pair.second.get() == this;
            }), siblings.end());
            p->markDirty();
            p->condenseTree();
        }
    }
}

bool RTreeNode::updateTrajectory(const Trajectory& traj) {
    if (deleteTrajectory(traj.id)) {
        insertRecursive(traj);
        return true;
    }
    return false;
}

void RTreeNode::findSimilar(const Trajectory& query, float maxDistance, std::vector<Trajectory>& results) const {
    BoundingBox3D queryBox = query.computeBoundingBox();
    if (!getMBR().intersects(queryBox)) return;
    if (isLeaf) {
        for (const auto& [box, traj] : leafEntries) {
            if (query.similarityTo(*traj) <= maxDistance) {
                results.push_back(*traj);
            }
        }
    } else {
        for (const auto& [box, child] : childEntries) {
            if (box.intersects(queryBox)) {
                child->findSimilar(query, maxDistance, results);
            }
        }
    }
}

std::vector<Trajectory> RTreeNode::kNearestNeighbors(const Trajectory& query, size_t k) const {
    auto comp = [](const ResultEntry& a, const ResultEntry& b) { return a.distance < b.distance; };
    std::priority_queue<ResultEntry, std::vector<ResultEntry>, decltype(comp)> knn(comp);
    std::priority_queue<HeapEntry> pq;

    pq.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    while (!pq.empty()) {
        auto [dist, node] = pq.top(); pq.pop();
        if (node->isLeaf) {
            for (const auto& [box, traj] : node->leafEntries) {
                float d = query.similarityTo(*traj);
                knn.push({d, *traj});
                if (knn.size() > k) knn.pop();
            }
        } else {
            for (const auto& [box, child] : node->childEntries) {
                float d = query.computeBoundingBox().distanceTo(box);
                pq.push({d, child});
            }
        }
    }

    std::vector<Trajectory> results;
    while (!knn.empty()) {
        results.push_back(knn.top().traj);
        knn.pop();
    }
    std::reverse(results.begin(), results.end());
    return results;
}

json RTreeNode::to_json() const {
    json j;
    j["mbr"] = getMBR().to_json();
    j["leaf"] = isLeaf;
    j["entries"] = json::array();

    if (isLeaf) {
        for (const auto& [box, traj] : leafEntries) {
            j["entries"].push_back({
                {"box", box.to_json()},
                {"trajectory", traj->to_json()}
            });
        }
    } else {
        for (const auto& [box, child] : childEntries) {
            j["entries"].push_back({
                {"box", box.to_json()},
                {"child", child->to_json()}
            });
        }
    }
    return j;
}

void RTreeNode::insertLeafEntry(const BoundingBox3D& box, std::shared_ptr<Trajectory> traj) {
    if (!isLeaf) throw std::runtime_error("insertLeafEntry called on non-leaf node");
    leafEntries.emplace_back(box, traj);
    markDirty();
}

const std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& RTreeNode::getLeafEntries() const {
    return leafEntries;
}

const std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>& RTreeNode::getChildEntries() const {
    return childEntries;
}
