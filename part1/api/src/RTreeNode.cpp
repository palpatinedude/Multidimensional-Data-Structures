#include "../include/RTreeNode.h"
#include "../include/splitHelpers.inl"
#include <limits>
#include <algorithm>
#include <iostream>
#include <queue>
#include <cmath>
#include <unordered_set>

using namespace splitHelpers;


// Helper structs for kNN search
struct HeapEntry {
    float distSq;  // distance squared from query to this node
    std::shared_ptr<RTreeNode> node;  // pointer to the R-tree node
    // Min-heap: nodes with smaller distSq have higher priority
    bool operator<(const HeapEntry& other) const { return distSq > other.distSq; }
};

struct ResultEntry {
    float distSq;  // distance squared from query to this trajectory
    Trajectory traj;  // the trajectory itself
    // Max-heap: farther trajectories have higher priority (for kNN pruning)
    bool operator<(const ResultEntry& other) const { return distSq < other.distSq; }
};


// ---------------- Constructors ----------------
 // Initialize node as leaf/internal with maxEntries

RTreeNode::RTreeNode(bool isLeaf, int maxEntries)
    : isLeaf(isLeaf), maxEntries(maxEntries), mbr_dirty(true) {
}  

// ---------------- Node Info ----------------
bool RTreeNode::isLeafNode() const { return isLeaf; }

bool RTreeNode::isEmpty() const {
    // Check if node has no entries
    return isLeaf ? leafEntries.empty() : childEntries.empty();
}

// Check if node has exceeded maxEntries
bool RTreeNode::needsSplit() const {
    
    return isLeaf ? leafEntries.size() > static_cast<size_t>(maxEntries) : childEntries.size() > static_cast<size_t>(maxEntries);
}

// Recompute MBR if dirty
BoundingBox3D RTreeNode::getMBR() const {
    
    if (mbr_dirty) updateMBR();
    return mbr;
}

// ---------------- MBR Management ----------------
 // Mark current MBR as dirty and propagate upwards
void RTreeNode::markDirty() {
   
    mbr_dirty = true;
    if (auto p = parent.lock()) p->markDirty();
}

// Recompute MBR from all entries or child nodes
void RTreeNode::updateMBR() const {
    
    mbr = BoundingBox3D();

    if (isLeaf) {
        // Combine all bounding boxes of trajectories
        for (const auto& [box, _] : leafEntries)
            mbr.expandToInclude(box);
    } else {
        // Combine all child nodes' MBRs
        for (const auto& [box, _] : childEntries)
            mbr.expandToInclude(box);
    }

    mbr_dirty = false;  // MBR is now up-to-date
}

 // Recursively recompute MBRs for all children
void RTreeNode::recomputeMBRs() {
   
    if (!isLeaf) {
        for (auto& [_, child] : childEntries)
            child->recomputeMBRs();
    }
    // Update this node's MBR after children
    updateMBR();
}

 // Compute how much current MBR would grow to include 'toInclude'
float RTreeNode::enlargement(const BoundingBox3D& current, const BoundingBox3D& toInclude) const {
   
    BoundingBox3D combined = current;
    combined.expandToInclude(toInclude);
    return combined.volume() - current.volume();
}

int RTreeNode::chooseSubtree(const BoundingBox3D& box) const {
    // Select best child to insert a trajectory
    if (childEntries.empty()) return -1;

    float minEnlargement = std::numeric_limits<float>::max();
    float minArea = std::numeric_limits<float>::max();
    int bestIndex = -1;

    for (size_t i = 0; i < childEntries.size(); ++i) {
        float enlarge = enlargement(childEntries[i].first, box);
        float area = childEntries[i].first.volume();
        if (enlarge < minEnlargement || (enlarge == minEnlargement && area < minArea)) {
            minEnlargement = enlarge;
            minArea = area;
            bestIndex = static_cast<int>(i);
        }
    }
    return bestIndex;
}

// ---------------- Insertion ----------------

// Recursively insert trajectory into tree
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::insertRecursive(const Trajectory &traj) {
    return isLeaf ? insertIntoLeaf(traj) : insertIntoInternal(traj);
}

// Insert trajectory into a leaf node
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::insertIntoLeaf(const Trajectory &traj) {
    insertLeaf(traj.getBoundingBox(), std::make_shared<Trajectory>(traj));

    // Split node if overfull
    if ((int)leafEntries.size() > maxEntries) return splitLeaf();

    return {nullptr, nullptr};
}

// Insert trajectory into internal node
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::insertIntoInternal(const Trajectory &traj) {
    int bestChildIndex = chooseSubtree(traj.getBoundingBox());
    auto [splitLeft, splitRight] = childEntries[bestChildIndex].second->insertRecursive(traj);

    // If child was split, replace child with split nodes
    if (splitLeft && splitRight) {
        childEntries.erase(childEntries.begin() + bestChildIndex);
        insertChild(splitLeft->getMBR(), splitLeft);
        insertChild(splitRight->getMBR(), splitRight);

        // Split internal node if needed
        if ((int)childEntries.size() > maxEntries) return splitInternal();
    }

    updateMBR();  // Update this node's MBR
    return {nullptr, nullptr};
}

// ---------------- Node Splitting ----------------
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitLeaf() {
    // Split leaf node using quadratic split
    auto left = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    quadraticSplitEntries(leafEntries, left, right, maxEntries);

    leafEntries.clear();  // Clear old entries
    return {left, right};
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitInternal() {
    // Split internal node using quadratic split
    auto left = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    quadraticSplitEntries(childEntries, left, right, maxEntries);

    childEntries.clear();  // Clear old entries
    return {left, right};
}

// Insert child/internal node
void RTreeNode::insertChild(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child) {
    if (!child) throw std::runtime_error("Attempting to insert nullptr child");

    child->parent = shared_from_this();       // Set parent pointer
    childEntries.emplace_back(box, child);    // Add child entry
    markDirty();                              // Mark MBR dirty
}

// Insert trajectory into leaf
void RTreeNode::insertLeaf(const BoundingBox3D& box, std::shared_ptr<Trajectory> traj) {
    if (!isLeaf) throw std::runtime_error("insertLeaf called on non-leaf node");

    leafEntries.emplace_back(box, traj);      // Add trajectory entry
    markDirty();                              // Mark MBR dirty
}

// ---------------- Queries ----------------
void RTreeNode::rangeQuery(const BoundingBox3D& queryBox, std::vector<Trajectory>& results) const {
    // Skip node if MBR does not intersect query
    if (!getMBR().intersects(queryBox)) return;

    if (isLeaf) {
        // Leaf: check each trajectory
        for (const auto& [box, traj] : leafEntries)
            if (queryBox.intersects(box)) results.push_back(*traj);
    } else {
        // Internal: recurse into children
        for (const auto& [box, child] : childEntries)
            if (queryBox.intersects(box)) child->rangeQuery(queryBox, results);
    }
}

// Find similar trajectories within threshold
void RTreeNode::findSimilar(const Trajectory& query, float maxDistance, std::vector<Trajectory>& results) const {
    BoundingBox3D queryBox = query.computeBoundingBox();

    // Skip node if MBR does not intersect
    if (!getMBR().intersects(queryBox)) return;

    if (isLeaf) {
        // Check each trajectory
        for (const auto& [_, traj] : leafEntries)
            if (query.similarityTo(*traj) <= maxDistance)
                results.push_back(*traj);
    } else {
        // Recurse into children
        for (const auto& [box, child] : childEntries)
            if (box.intersects(queryBox)) child->findSimilar(query, maxDistance, results);
    }
}
/*
// Find the k nearest trajectories to the query
std::vector<Trajectory> RTreeNode::kNearestNeighbors(const Trajectory& query, size_t k) const {

    std::priority_queue<ResultEntry> knn;  // max-heap storing current k nearest trajectories
    std::priority_queue<HeapEntry> pq;     // min-heap storing nodes to explore

    BoundingBox3D queryBox = query.computeBoundingBox(); // compute bounding box of query once

    // Start with the root node, distance = 0
    pq.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    // Lambda to get the farthest distance in the current knn heap
    auto getFarthestDistSq = [&]() -> float {
        return knn.empty() ? std::numeric_limits<float>::max() : knn.top().distSq;
    };

    while (!pq.empty()) {
        // Pop the node closest to query so far
        auto [distSqNode, node] = pq.top(); 
        pq.pop();

        // If closest node is farther than farthest in knn, we can stop
        if (distSqNode > getFarthestDistSq()) break;

        if (node->isLeaf) {
            // Leaf node: check all trajectories
            for (const auto& [_, trajPtr] : node->leafEntries) {
                float distSq = query.spatioTemporalDistanceTo(*trajPtr); // squared distance to trajectory

                // If heap not full or found a closer trajectory, add it
                if (knn.size() < k || distSq < getFarthestDistSq()) {
                    knn.push({distSq, *trajPtr});
                    // Keep heap size <= k
                    if (knn.size() > k) knn.pop();
                }
            }
        } else {
            // Internal node: check all children
            for (const auto& [childBox, child] : node->childEntries) {
                float minDistSq = queryBox.distanceSquaredTo(childBox); // min distance to child box

                // Only explore child if it could contain a closer trajectory
                if (minDistSq <= getFarthestDistSq()) {
                    pq.push({minDistSq, child});
                }
            }
        }
    }

    // Extract results from heap into vector
    std::vector<Trajectory> results;
    while (!knn.empty()) {
        results.push_back(knn.top().traj);
        knn.pop();
    }

    // Reverse because we want nearest first
    std::reverse(results.begin(), results.end());
    return results;
}

*/

std::vector<Trajectory> RTreeNode::kNearestNeighbors(const Trajectory& query, size_t k, float timeScale, size_t candidateMultiplier) const
{
    std::cout << "beggining of RTreeNode::kNearestNeighbors"<<std::endl;
    using ResultPair = std::pair<float, Trajectory>; // distance, trajectory
    std::priority_queue<ResultEntry> knn;           // max-heap for candidate distances
    std::priority_queue<HeapEntry> pq;              // min-heap for nodes to explore

    BoundingBox3D queryBox = query.computeBoundingBox();
    pq.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    // Keep more candidates than k to ensure enough unique vehicles
    size_t kCandidates = k * candidateMultiplier;

    auto getFarthestDist = [&]() -> float {
        return knn.empty() ? std::numeric_limits<float>::max() : knn.top().distSq;
    };

    std::cout<< " while (!pq.empty()) " <<std::endl;
    while (!pq.empty()) {
        auto [distNode, node] = pq.top(); pq.pop();

        if (distNode > getFarthestDist()) break;

        if (node->isLeaf) {
            for (const auto& [_, trajPtr] : node->leafEntries) {
                float dist = query.spatioTemporalDistanceTo(*trajPtr, timeScale);

                if (knn.size() < kCandidates || dist < getFarthestDist()) {
                    knn.push({dist, *trajPtr});
                    if (knn.size() > kCandidates) knn.pop();
                }
            }
        } else {
            for (const auto& [childBox, child] : node->childEntries) {
                float minDist = queryBox.distanceSquaredTo(childBox);
                if (minDist <= getFarthestDist()) pq.push({minDist, child});
            }
        }
    }

    std::cout<<"before extract candidates"<<std::endl;

    // Extract candidates and filter by unique vehicle
    std::vector<ResultPair> candidates;
    while (!knn.empty()) {
        candidates.push_back({knn.top().distSq, knn.top().traj});
        knn.pop();
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const ResultPair& a, const ResultPair& b){ return a.first < b.first; });

    std::vector<Trajectory> results;
    std::unordered_set<std::string> seenTrajIds;

    for (auto& [dist, traj] : candidates) {
        std::string trajId = traj.getId(); // assumes you have gettrajId()
        if (trajId == query.getId() || seenTrajIds.count(trajId)) continue;
        results.push_back(traj);
        seenTrajIds.insert(trajId);
        if (results.size() >= k) break;
    }

    return results;
}


// ---------------- Deletion & Update ----------------
bool RTreeNode::deleteTrajectory(const std::string& trajId) {
    if (isLeaf) {
        // Remove trajectory from leaf
        auto it = std::remove_if(leafEntries.begin(), leafEntries.end(),
                                 [&](const auto& pair){ return pair.second->getId() == trajId; });
        if (it != leafEntries.end()) {
            leafEntries.erase(it, leafEntries.end());
            markDirty();        // Update MBR
            condenseTree();     // Maintain tree structure
            return true;
        }
        return false;
    } else {
        // Recurse into children
        for (auto& [_, child] : childEntries)
            if (child->deleteTrajectory(trajId)) { markDirty(); return true; }
        return false;
    }
}


bool RTreeNode::updateTrajectory(const Trajectory& traj) {
    if (!isLeaf) {
        // Recurse into children
        for (auto& [_, child] : childEntries) {
            if (child->updateTrajectory(traj)) {
                updateMBR(); // Update MBR if child changed
                return true;
            }
        }
        return false;
    }

    // Leaf node
    for (auto& [bbox, trajPtr] : leafEntries) {
        if (trajPtr->getId() == traj.getId()) {
            BoundingBox3D newBox = traj.getBoundingBox();
            // Check if the updated trajectory fits within the existing leaf MBR
            if (mbr.intersects(newBox)) {
                // Replace trajectory in place
                *trajPtr = traj;
                markDirty(); // MBR may still need update
                return true;
            } else {
                // Remove and reinsert elsewhere
                leafEntries.erase(std::remove_if(
                    leafEntries.begin(), leafEntries.end(),
                    [&](const auto& pair) { return pair.second->getId() == traj.getId(); }),
                    leafEntries.end()
                );
                markDirty();
                return false; // Signal that it needs reinsertion
            }
        }
    }
    return false; // Trajectory not found
}


// ---------------- Condense Tree ----------------
void RTreeNode::condenseTree() {
    auto p = parent.lock();
    if (!p) return; // reached root

    std::vector<std::shared_ptr<RTreeNode>> toReinsert;

    if (isLeaf) {
        if (leafEntries.empty()) removeFromParent();
    } else {
        if (childEntries.empty()) removeFromParent();
        else if ((int)childEntries.size() < (maxEntries + 1)/2) {
            // Collect orphaned children to reinsert
            for (auto& [_, child] : childEntries) toReinsert.push_back(child);
            childEntries.clear();
            removeFromParent();
        }
    }

    if (p) {
        p->condenseTree();
        for (auto& child : toReinsert) p->insertChild(child->getMBR(), child);
    }
}

bool RTreeNode::operator!=(const RTreeNode& other) const {
    return !(*this == other);
}

bool RTreeNode::operator==(const RTreeNode& other) const {
    if (isLeaf != other.isLeaf || 
        maxEntries != other.maxEntries || 
        mbr_dirty != other.mbr_dirty) {
        return false;
    }
    if (mbr != other.mbr) return false;

    if (isLeaf) {
        return leafEntries == other.leafEntries;
    } else {
        return childEntries == other.childEntries;
    }
}

json RTreeNode::to_json() const {
    json j;
    j["isLeaf"] = isLeaf;

    if (isLeaf) {
        // Just record bounding boxes, no trajectories
        j["entries"] = json::array();
        for (const auto& [box, traj] : leafEntries) {
            j["entries"].push_back({
                {"box", box.to_json()}
                // removed trajectory export
            });
        }
    } else {
        // Internal node: export children recursively with their MBRs
        j["children"] = json::array();
        for (const auto& [childBox, childNode] : childEntries) {
            j["children"].push_back({
                {"box", childBox.to_json()},
                {"node", childNode->to_json()}
            });
        }
    }

    return j;
}


// Remove node from parent
void RTreeNode::removeFromParent() {
    if (auto p = parent.lock()) {
        auto& siblings = p->childEntries;
        siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                                      [&](const auto& pair){ return pair.second.get() == this; }),
                       siblings.end());
        p->markDirty();
    }
}

// Getters 
const std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& RTreeNode::getLeafEntries() const{
    return leafEntries;
}

const std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>& RTreeNode::getChildEntries() const{
    return childEntries;
}

/*
bool RTreeNode::updateTrajectory(const Trajectory& traj) {
    // Delete old trajectory and insert new
    bool deleted = deleteTrajectory(traj.getId());
    if (!deleted) { std::cerr << "Warning: update failed ID=" << traj.getId() << std::endl; return false; }
    insertRecursive(traj);
    return true;
}*/

/*
std::vector<Trajectory> RTreeNode::kNearestNeighbors(const Trajectory& query, size_t k) const {
    std::priority_queue<ResultEntry> knn;  // Max-heap of current nearest trajectories
    std::priority_queue<HeapEntry> pq;     // Min-heap of nodes to explore by MBR distance

    // Start search from this node
    pq.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    while (!pq.empty()) {
        auto [distSq, node] = pq.top();
        pq.pop();

        // Skip node if it cannot contain a closer trajectory
        if (!knn.empty() && distSq > knn.top().distance * knn.top().distance)
            continue;

        if (node->isLeaf) {
            // Evaluate all trajectories in leaf node
            for (const auto& [box, traj] : node->leafEntries) {
                float dist = query.similarityTo(*traj);
                if (knn.size() < k || dist < knn.top().distance) {
                    knn.push({dist, *traj});  // Add to nearest neighbors
                    if (knn.size() > k) knn.pop(); // Keep only k closest
                }
            }
        } else {
            // Add child nodes to queue for exploration
            for (const auto& [box, child] : node->childEntries) {
                float distSqChild = query.computeBoundingBox().distanceSquaredTo(box);
                pq.push({distSqChild, child});
            }
        }
    }

    // Extract results from max-heap (closest first)
    std::vector<Trajectory> results;
    while (!knn.empty()) {
        results.push_back(knn.top().traj);
        knn.pop();
    }
    std::reverse(results.begin(), results.end());
    return results;
}*/



/*
struct HeapEntry {
    float distanceSquared;  // squared distance for bounding boxes
    std::shared_ptr<RTreeNode> node;
    bool operator<(const HeapEntry& other) const {
        // Min-heap: smaller squared distance = higher priority
        return distanceSquared > other.distanceSquared;
    }
};

struct ResultEntry {
    float distance;  // actual distance (similarity measure)
    Trajectory traj;
    bool operator<(const ResultEntry& other) const {
        // Max-heap: largest distance at top, to pop when size > k
        return distance < other.distance;
    }
};


RTreeNode::RTreeNode(bool isLeaf, int maxEntries)
    : isLeaf(isLeaf), maxEntries(maxEntries), mbr_dirty(true) {}

bool RTreeNode::isLeafNode() const {
    return isLeaf;
}

BoundingBox3D RTreeNode::getMBR() const {

    if (mbr_dirty) {
        updateMBR();
    }
    return mbr;
}

void RTreeNode::updateMBR() const {
    mbr = BoundingBox3D();
    if (isLeaf) {
        for (size_t i = 0; i < leafEntries.size(); ++i) {
            const auto &box = leafEntries[i].first;
            mbr.expandToInclude(box);
        }
    } else {
        for (size_t i = 0; i < childEntries.size(); ++i) {
            const auto &box = childEntries[i].first;
            mbr.expandToInclude(box);
        }
    }
    mbr_dirty = false;
}

void RTreeNode::markDirty() {
    mbr_dirty = true;
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

    if (childEntries.empty()) {
        return -1;
    }
    float minEnlargement = std::numeric_limits<float>::max();
    float minArea = std::numeric_limits<float>::max();

    int bestIndex = -1;
    for (size_t i = 0; i < childEntries.size(); ++i) {
        float enlarge = enlargement(childEntries[i].first, box);
        float area = childEntries[i].first.volume();
        if (enlarge < minEnlargement || 
            (enlarge == minEnlargement && area < minArea)) {
             minEnlargement = enlarge;
             minArea = area;
                bestIndex = static_cast<int>(i);
        }   
    }
    return bestIndex;
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::insertRecursive(const Trajectory &traj) {
    if (isLeaf) {
        return insertIntoLeaf(traj);
    } else {
        return insertIntoInternal(traj);
    }
}

// --- helper for inserting into a leaf node ---
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::insertIntoLeaf(const Trajectory &traj) {
    insertLeaf(traj.getBoundingBox(), std::make_shared<Trajectory>(traj));

    if ((int)leafEntries.size() > maxEntries) {
        return splitLeaf();
    }
    return {nullptr, nullptr};
}

// --- helper for inserting into an internal node ---
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::insertIntoInternal(const Trajectory &traj) {
    int bestChildIndex = chooseSubtree(traj.getBoundingBox());
    auto [splitLeft, splitRight] = childEntries[bestChildIndex].second->insertRecursive(traj);

    if (splitLeft && splitRight) {
        // Remove old child
        childEntries.erase(childEntries.begin() + bestChildIndex);

        // Add new split children using insertChild()
        insertChild(splitLeft->getMBR(), splitLeft);
        insertChild(splitRight->getMBR(), splitRight);

        if ((int)childEntries.size() > maxEntries) {
            return splitInternal();
        }
    }

    updateMBR();
    return {nullptr, nullptr};
}




std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::splitLeaf() {
    auto left  = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    quadraticSplitEntries(leafEntries, left, right, maxEntries);

    leafEntries.clear();
    return {left, right};
}



std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::splitInternal() {
    auto left  = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    quadraticSplitEntries(childEntries, left, right, maxEntries);

    childEntries.clear();
    return {left, right};
}

void RTreeNode::insertChild(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child) {
    if (!child) {
        throw std::runtime_error("Attempting to insert nullptr child into RTreeNode");
    }
    child->parent = shared_from_this();
    childEntries.emplace_back(box, child);
    markDirty();
}

void RTreeNode::insertLeaf(const BoundingBox3D& box, std::shared_ptr<Trajectory> traj) {
    if (!isLeaf) throw std::runtime_error("insertLeafEntry called on non-leaf node");
    leafEntries.emplace_back(box, traj);
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

bool RTreeNode::deleteTrajectory(const std::string& trajId) {
    if (isLeaf) {
        auto it = std::remove_if(leafEntries.begin(), leafEntries.end(), [&](const auto& pair) {
            return pair.second->getId() == trajId;
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
    auto p = parent.lock();
    if (!p) return; // reached root

    std::vector<std::shared_ptr<RTreeNode>> toReinsert;

    if (isLeaf) {
        if (leafEntries.empty()) {
            removeFromParent();
        }
    } else {
        if (childEntries.empty()) {
            removeFromParent();
        } else if ((int)childEntries.size() < (maxEntries + 1) / 2) {
            // Orphans collected
            for (auto& [box, child] : childEntries) {
                toReinsert.push_back(child);
            }
            childEntries.clear();
            removeFromParent();
        }
    }

    if (p) {
        p->condenseTree();

        // Reinsert orphaned subtrees into parent after condensing
        for (auto& child : toReinsert) {
            p->insertChild(child->getMBR(), child);
        }
    }
}

void RTreeNode::removeFromParent() {
    if (auto p = parent.lock()) {
        auto& siblings = p->childEntries;
        siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
            [&](const auto& pair) { return pair.second.get() == this; }),
            siblings.end());
        p->markDirty();
    }
}

bool RTreeNode::updateTrajectory(const Trajectory& traj) {
    bool deleted = deleteTrajectory(traj.getId());
    if (!deleted) {
        std::cerr << "Warning: updateTrajectory failed to delete trajectory ID: " << traj.getId() << std::endl;
        return false;
    }
    insertRecursive(traj);
    return true;
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

void RTreeNode::setParent(std::shared_ptr<RTreeNode> p) {
    parent = p;
}

bool RTreeNode::isEmpty() const {
    return isLeaf ? leafEntries.empty() : childEntries.empty();
}


std::vector<Trajectory> RTreeNode::kNearestNeighbors(const Trajectory& query, size_t k) const {
    std::priority_queue<ResultEntry> knn;  // Max-heap for trajectories
    std::priority_queue<HeapEntry> pq;     // Min-heap for nodes by squared distance

    pq.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    while (!pq.empty()) {
        auto [distSq, node] = pq.top();
        pq.pop();

        // Prune: if this node’s MBR is farther than worst knn distance, skip
        if (!knn.empty() && distSq > knn.top().distance * knn.top().distance) {
            continue;
        }

        if (node->isLeaf) {
            for (const auto& [box, traj] : node->leafEntries) {
                float actualDist = query.similarityTo(*traj);
                if (knn.size() < k || actualDist < knn.top().distance) {
                    knn.push({actualDist, *traj});
                    if (knn.size() > k) knn.pop();
                }
            }
        } else {
            for (const auto& [box, child] : node->childEntries) {
                float distSquared = query.computeBoundingBox().distanceSquaredTo(box);
                pq.push({distSquared, child});
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
    j["isLeaf"] = isLeaf;

    if (isLeaf) {
        // Just record bounding boxes, no trajectories
        j["entries"] = json::array();
        for (const auto& [box, traj] : leafEntries) {
            j["entries"].push_back({
                {"box", box.to_json()}
                // removed trajectory export
            });
        }
    } else {
        // Internal node: export children recursively with their MBRs
        j["children"] = json::array();
        for (const auto& [childBox, childNode] : childEntries) {
            j["children"].push_back({
                {"box", childBox.to_json()},
                {"node", childNode->to_json()}
            });
        }
    }

    return j;
}

const std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& RTreeNode::getLeafEntries() const{
    return leafEntries;
}

const std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>& RTreeNode::getChildEntries() const{
    return childEntries;
}

void RTreeNode::recomputeMBRs() {
    if (!isLeaf) {
        for (auto& [_, child] : childEntries) {
            child->recomputeMBRs();  // Recurse first
        }
    }
    updateMBR();  // Then recompute this node’s MBR
}

bool RTreeNode::operator!=(const RTreeNode& other) const {
    return !(*this == other);
}

bool RTreeNode::operator==(const RTreeNode& other) const {
    if (isLeaf != other.isLeaf || 
        maxEntries != other.maxEntries || 
        mbr_dirty != other.mbr_dirty) {
        return false;
    }
    if (mbr != other.mbr) return false;

    if (isLeaf) {
        return leafEntries == other.leafEntries;
    } else {
        return childEntries == other.childEntries;
    }
}*/

/*
// Pick the two most wasteful seeds for quadratic split
template <typename EntryType>
static std::pair<int, int> RTreeNode::pickSeeds(const std::vector<EntryType>& entries) {
    int total = entries.size();
    int seed1 = -1, seed2 = -1;
    float worstWaste = -1.0f;

    // Compare each pair and pick the pair that wastes most volume
    for (int i = 0; i < total; ++i) {
        for (int j = i + 1; j < total; ++j) {
            BoundingBox3D combined = entries[i].first;
            combined.expandToInclude(entries[j].first);
            float waste = combined.volume() - entries[i].first.volume() - entries[j].first.volume();
            if (waste > worstWaste) {
                worstWaste = waste;
                seed1 = i;
                seed2 = j;
            }
        }
    }
    return {seed1, seed2};
}

// Assign an entry to the given node (leaf or internal) using compile-time type check
template <typename EntryType>
static void RTreeNode:: assignEntryToNode(std::shared_ptr<RTreeNode> node, const EntryType& entry) {
    if constexpr (std::is_same_v<EntryType, std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>) {
        node->insertLeaf(entry.first, entry.second);   // Leaf entry
    } else if constexpr (std::is_same_v<EntryType, std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>) {
        node->insertChild(entry.first, entry.second);  // Internal node entry
    }
}

// Force-assign entries to nodes if one node needs entries to satisfy minimum fill
template <typename EntryType>
static bool RTreeNode:: assignForcedEntries(std::shared_ptr<RTreeNode> left,
                               std::shared_ptr<RTreeNode> right,
                               const std::vector<EntryType>& entries,
                               std::vector<bool>& assigned,
                               int minFill) 
{
    int total = entries.size();
    int leftSize  = left->isLeafNode() ? left->getLeafEntries().size() : left->getChildEntries().size();
    int rightSize = right->isLeafNode() ? right->getLeafEntries().size() : right->getChildEntries().size();
    int remaining = total - std::count(assigned.begin(), assigned.end(), true);

    // Assign all remaining entries to left if left cannot reach minFill otherwise
    if (leftSize + remaining <= minFill) {
        for (int i = 0; i < total; ++i) {
            if (!assigned[i]) {
                assignEntryToNode(left, entries[i]);
                assigned[i] = true;
            }
        }
        return true;
    }

    // Assign all remaining entries to right if right cannot reach minFill otherwise
    if (rightSize + remaining <= minFill) {
        for (int i = 0; i < total; ++i) {
            if (!assigned[i]) {
                assignEntryToNode(right, entries[i]);
                assigned[i] = true;
            }
        }
        return true;
    }

    return false;
}

// Pick the next entry to assign based on largest MBR enlargement difference
template <typename EntryType>
static int RTreeNode::pickNextEntry(std::shared_ptr<RTreeNode> left,
                         std::shared_ptr<RTreeNode> right,
                         const std::vector<EntryType>& entries,
                         const std::vector<bool>& assigned,
                         bool& assignToLeft) 
{
    int total = entries.size();
    int next = -1;
    float maxDiff = -1.0f;

    for (int i = 0; i < total; ++i) {
        if (!assigned[i]) {
            float enlargeLeft  = left->enlargement(left->getMBR(), entries[i].first);
            float enlargeRight = right->enlargement(right->getMBR(), entries[i].first);
            float diff = std::fabs(enlargeLeft - enlargeRight);

            if (diff > maxDiff) {
                maxDiff = diff;
                next = i;
                // Tie-breaking: assign to the node with smaller enlargement or smaller MBR volume
                assignToLeft = (enlargeLeft < enlargeRight) ||
                               (enlargeLeft == enlargeRight && left->getMBR().volume() <= right->getMBR().volume());
            }
        }
    }

    return next;
}

// Perform quadratic split of entries into two nodes
template <typename EntryType>
static void RTreeNode::quadraticSplitEntries(const std::vector<EntryType>& entries, 
                                  std::shared_ptr<RTreeNode> leftNode,
                                  std::shared_ptr<RTreeNode> rightNode,
                                  int maxEntries) 
{
    int minFill = (maxEntries + 1) / 2;

    // Step 1: Pick seeds
    auto [seed1, seed2] = pickSeeds(entries);
    assignEntryToNode(leftNode, entries[seed1]);
    assignEntryToNode(rightNode, entries[seed2]);

    // Track assigned entries
    std::vector<bool> assigned(entries.size(), false);
    assigned[seed1] = assigned[seed2] = true;

    // Step 2: Distribute remaining entries
    while (std::count(assigned.begin(), assigned.end(), true) < entries.size()) {
        // Force assignment if minFill requires
        if (assignForcedEntries(leftNode, rightNode, entries, assigned, minFill)) break;

        // Pick next best entry to assign
        bool assignToLeft = false;
        int next = pickNextEntry(leftNode, rightNode, entries, assigned, assignToLeft);
        assignEntryToNode(assignToLeft ? leftNode : rightNode, entries[next]);
        assigned[next] = true;
    }

    // Step 3: Update MBRs
    leftNode->updateMBR();
    rightNode->updateMBR();
}
*/

/*
void RTreeNode::condenseTree() {
    std::vector<std::shared_ptr<RTreeNode>> toReinsert;

    auto p = parent.lock();
    if (!p) return; // root reached

    if (isLeaf) {
        if (leafEntries.empty()) {
            // Remove this node from parent
            auto& siblings = p->childEntries;
            siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                [&](const auto& pair) { return pair.second.get() == this; }),
                siblings.end());
        }
    } else {
        if (childEntries.empty()) {
            // Remove empty internal node
            auto& siblings = p->childEntries;
            siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                [&](const auto& pair) { return pair.second.get() == this; }),
                siblings.end());
        } else if ((int)childEntries.size() < (maxEntries + 1) / 2) {
            // Underfilled: reinsert its children into parent
            for (auto& [box, child] : childEntries) {
                toReinsert.push_back(child);
            }
            childEntries.clear();

            auto& siblings = p->childEntries;
            siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                [&](const auto& pair) { return pair.second.get() == this; }),
                siblings.end());
        }
    }

    p->markDirty();
    p->condenseTree();

    // Reinsert orphaned subtrees into the parent
    for (auto& child : toReinsert) {
        p->insertChild(child->getMBR(), child);
    }
}*/

/*
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::splitInternal() {
    int seed1 = 0, seed2 = 1;

    auto left  = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    // Insert seeds using insertChild()
    left->insertChild(childEntries[seed1].first, childEntries[seed1].second);
    right->insertChild(childEntries[seed2].first, childEntries[seed2].second);

    std::vector<bool> used(childEntries.size(), false);
    used[seed1] = used[seed2] = true;

    // Distribute remaining entries
    for (size_t i = 0; i < childEntries.size(); ++i) {
        if (used[i]) continue;
        const auto& entry = childEntries[i];

        float enlargeLeft  = enlargement(left->getMBR(), entry.first);
        float enlargeRight = enlargement(right->getMBR(), entry.first);

        if (enlargeLeft < enlargeRight || right->childEntries.empty()) {
            left->insertChild(entry.first, entry.second);
        } else if (enlargeRight < enlargeLeft || left->childEntries.empty()) {
            right->insertChild(entry.first, entry.second);
        } else if (left->getMBR().volume() <= right->getMBR().volume()) {
            left->insertChild(entry.first, entry.second);
        } else {
            right->insertChild(entry.first, entry.second);
        }
    }

    left->updateMBR();
    right->updateMBR();
    childEntries.clear();

    return { left, right };
}


std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::splitLeaf() {
    int seed1 = 0, seed2 = 1;

    auto left  = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    // Insert seeds using insertLeaf()
    left->insertLeaf(leafEntries[seed1].first, leafEntries[seed1].second);
    right->insertLeaf(leafEntries[seed2].first, leafEntries[seed2].second);

    std::vector<bool> used(leafEntries.size(), false);
    used[seed1] = used[seed2] = true;

    // Distribute remaining entries
    for (size_t i = 0; i < leafEntries.size(); ++i) {
        if (used[i]) continue;
        const auto& entry = leafEntries[i];

        float enlargeLeft  = enlargement(left->getMBR(), entry.first);
        float enlargeRight = enlargement(right->getMBR(), entry.first);

        if (enlargeLeft < enlargeRight || right->leafEntries.empty()) {
            left->insertLeaf(entry.first, entry.second);
        } else if (enlargeRight < enlargeLeft || left->leafEntries.empty()) {
            right->insertLeaf(entry.first, entry.second);
        } else if (left->getMBR().volume() <= right->getMBR().volume()) {
            left->insertLeaf(entry.first, entry.second);
        } else {
            right->insertLeaf(entry.first, entry.second);
        }
    }

    left->updateMBR();
    right->updateMBR();
    leafEntries.clear();

    return { left, right };
}
*/




/*
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitLeaf() {
    int seed1 = 0, seed2 = 1;

    auto left  = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    left->leafEntries.push_back(leafEntries[seed1]);
    right->leafEntries.push_back(leafEntries[seed2]);

    std::vector<bool> used(leafEntries.size(), false);
    used[seed1] = used[seed2] = true;

    for (size_t i = 0; i < leafEntries.size(); ++i) {
        if (used[i]) continue;
        const auto& entry = leafEntries[i];
        float enlargeLeft  = enlargement(left->getMBR(), entry.first);
        float enlargeRight = enlargement(right->getMBR(), entry.first);

        if (enlargeLeft < enlargeRight || right->leafEntries.empty())
            left->leafEntries.push_back(entry);
        else if (enlargeRight < enlargeLeft || left->leafEntries.empty())
            right->leafEntries.push_back(entry);
        else if (left->getMBR().volume() <= right->getMBR().volume())
            left->leafEntries.push_back(entry);
        else
            right->leafEntries.push_back(entry);
    }

    left->updateMBR();
    right->updateMBR();
    leafEntries.clear();  // optional: node is being replaced

    return { left, right }; // always return both nodes
}






std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitInternal() {
    int seed1 = 0, seed2 = 1;

    auto left  = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    left->childEntries.push_back(childEntries[seed1]);
    if (childEntries[seed1].second) childEntries[seed1].second->parent = left;

    right->childEntries.push_back(childEntries[seed2]);
    if (childEntries[seed2].second) right->childEntries.back().second->parent = right;

    std::vector<bool> used(childEntries.size(), false);
    used[seed1] = used[seed2] = true;

    for (size_t i = 0; i < childEntries.size(); ++i) {
        if (used[i]) continue;
        auto entry = childEntries[i];
        float enlargeLeft  = enlargement(left->getMBR(), entry.first);
        float enlargeRight = enlargement(right->getMBR(), entry.first);

        if (enlargeLeft < enlargeRight || right->childEntries.empty()) {
            left->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = left;
        } else if (enlargeRight < enlargeLeft || left->childEntries.empty()) {
            right->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = right;
        } else if (left->getMBR().volume() <= right->getMBR().volume()) {
            left->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = left;
        } else {
            right->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = right;
        }
    }

    left->updateMBR();
    right->updateMBR();
    childEntries.clear();

    return { left, right }; // always return both
}

*/


/*
// --- helper for inserting into a leaf node ---
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::insertIntoLeaf(const Trajectory &traj) {
    leafEntries.emplace_back(traj.getBoundingBox(), std::make_shared<Trajectory>(traj));
    markDirty();
    updateMBR();

    if ((int)leafEntries.size() > maxEntries) {
        return splitLeaf();
    }

    return {nullptr, nullptr};
}

// --- helper for inserting into an internal node ---
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::insertIntoInternal(const Trajectory &traj) {
    int bestChildIndex = chooseSubtree(traj.getBoundingBox());
    auto [splitLeft, splitRight] = childEntries[bestChildIndex].second->insertRecursive(traj);

    if (splitLeft && splitRight) {
        // Remove old child
        childEntries.erase(childEntries.begin() + bestChildIndex);
        // Add new split children
        childEntries.emplace_back(splitLeft->getMBR(), splitLeft);
        childEntries.emplace_back(splitRight->getMBR(), splitRight);
        splitLeft->parent = shared_from_this();
        splitRight->parent = shared_from_this();

        if ((int)childEntries.size() > maxEntries) {
            return splitInternal();
        }
    }

    updateMBR();
    return {nullptr, nullptr};
}

*/

/*
json RTreeNode::to_json() const {
    json j;
    j["isLeaf"] = isLeaf;

    if (isLeaf) {
        j["entries"] = json::array();
        for (const auto& [box, traj] : leafEntries) {
            j["entries"].push_back({
                {"box", box.to_json()},
                {"trajectory", traj->to_json()}
            });
        }
    } else {
        j["children"] = json::array();
        for (const auto& [childBox, childNode] : childEntries) {
            j["children"].push_back({
                {"box", childBox.to_json()},
                {"node", childNode->to_json()} // recursion here
            });
        }
    }

    return j;
}
*/


/*
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::insertRecursive(const Trajectory &traj) {

    if (isLeaf) {
        leafEntries.emplace_back(traj.getBoundingBox(), std::make_shared<Trajectory>(traj));
        markDirty();
        updateMBR();

        if ((int)leafEntries.size() > maxEntries) {
            return splitLeaf(); // return both nodes
        }
        return {nullptr, nullptr};
    } else {
        int bestChildIndex = chooseSubtree(traj.getBoundingBox());
        auto [splitLeft, splitRight] = childEntries[bestChildIndex].second->insertRecursive(traj);

        if (splitLeft && splitRight) {
            // Remove the old child
            childEntries.erase(childEntries.begin() + bestChildIndex);

            // Insert both new children
            childEntries.emplace_back(splitLeft->getMBR(), splitLeft);
            childEntries.emplace_back(splitRight->getMBR(), splitRight);

            splitLeft->parent = shared_from_this();
            splitRight->parent = shared_from_this();

            if ((int)childEntries.size() > maxEntries) {
                return splitInternal();
            }
        }

        updateMBR();
        return {nullptr, nullptr};
    }
}
*/


/*
std::shared_ptr<RTreeNode> RTreeNode::insertRecursive(const Trajectory& traj) {
    if (isLeaf) {
        leafEntries.emplace_back(traj.getBoundingBox(), std::make_shared<Trajectory>(traj));
        mbr_dirty = true;

        if ((int)leafEntries.size() > maxEntries) {
            auto [left, right] = splitLeaf();

            // **Parent should replace this node**
            if (parent.lock() == nullptr) { // Root split
                auto newRoot = std::make_shared<RTreeNode>(false, maxEntries);
                left->parent = newRoot;
                right->parent = newRoot;

                newRoot->childEntries.emplace_back(left->getMBR(), left);
                newRoot->childEntries.emplace_back(right->getMBR(), right);
                return newRoot;
            } else {
                // Return both new nodes to be inserted by parent
                return right;
            }
        }
        return nullptr;
    }

    // Internal node
    int idx = chooseSubtree(traj.getBoundingBox());
    auto child = childEntries[idx].second;
    auto splitNode = child->insertRecursive(traj);

    if (splitNode) {
        child->updateMBR();
        splitNode->updateMBR();

        // Replace old child with two new nodes
        childEntries[idx].first = child->getMBR();
        childEntries.insert(childEntries.begin() + idx + 1, {splitNode->getMBR(), splitNode});
        splitNode->parent = shared_from_this();

        if ((int)childEntries.size() > maxEntries) {
            auto [left, right] = splitInternal();

            if (parent.lock() == nullptr) { // Root split
                auto newRoot = std::make_shared<RTreeNode>(false, maxEntries);
                left->parent  = newRoot;
                right->parent = newRoot;

                newRoot->childEntries.emplace_back(left->getMBR(), left);
                newRoot->childEntries.emplace_back(right->getMBR(), right);
                return newRoot;
            } else {
                // Return right to be inserted by parent
                childEntries = left->childEntries;
                updateMBR();
                return right;
            }
        }
    } else {
        childEntries[idx].first = child->getMBR();
    }

    return nullptr;
}

*/

/*
std::shared_ptr<RTreeNode> RTreeNode::insertRecursive(const Trajectory &traj) {
    if (isLeaf) {
        leafEntries.emplace_back(traj.getBoundingBox(), std::make_shared<Trajectory>(traj));
        markDirty();
        updateMBR();

        if ((int)leafEntries.size() > maxEntries) {
            auto [left, right] = splitLeaf();

            if (left) left->parent = shared_from_this();
            if (right) right->parent = shared_from_this();

            // Return the sibling that should propagate
            return right ? right : left;
        }
        return nullptr;
    } else {
        int bestChildIndex = chooseSubtree(traj.getBoundingBox());
        auto splitNode = childEntries[bestChildIndex].second->insertRecursive(traj);

        if (splitNode) {
            splitNode->parent = shared_from_this();
            childEntries.emplace_back(splitNode->getMBR(), splitNode);
            markDirty();
            updateMBR();

            if ((int)childEntries.size() > maxEntries) {
                auto [left, right] = splitInternal();

                if (left) left->parent = shared_from_this();
                if (right) right->parent = shared_from_this();

                return right ? right : left;
            }
        }

        updateMBR();
        return nullptr;
    }
}


std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitLeaf() {
    int seed1 = 0, seed2 = 1;

    auto left  = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);

    left->leafEntries.push_back(leafEntries[seed1]);
    right->leafEntries.push_back(leafEntries[seed2]);

    std::vector<bool> used(leafEntries.size(), false);
    used[seed1] = used[seed2] = true;

    for (size_t i = 0; i < leafEntries.size(); ++i) {
        if (used[i]) continue;
        const auto& entry = leafEntries[i];
        float enlargeLeft  = enlargement(left->getMBR(), entry.first);
        float enlargeRight = enlargement(right->getMBR(), entry.first);

        if (enlargeLeft < enlargeRight || right->leafEntries.empty()) left->leafEntries.push_back(entry);
        else if (enlargeRight < enlargeLeft || left->leafEntries.empty()) right->leafEntries.push_back(entry);
        else if (left->getMBR().volume() <= right->getMBR().volume()) left->leafEntries.push_back(entry);
        else right->leafEntries.push_back(entry);
    }

    left->updateMBR();
    right->updateMBR();
    leafEntries.clear();

    // Ensure we never return empty nodes
    return { left->isEmpty() ? nullptr : left,
             right->isEmpty() ? nullptr : right };
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitInternal() {
    int seed1 = 0, seed2 = 1;

    auto left  = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);

    left->childEntries.push_back(childEntries[seed1]);
    if (childEntries[seed1].second) 
        childEntries[seed1].second->parent = left;

    right->childEntries.push_back(childEntries[seed2]);
    if (childEntries[seed2].second) 
        childEntries[seed2].second->parent = right;

    std::vector<bool> used(childEntries.size(), false);
    used[seed1] = used[seed2] = true;

    for (size_t i = 0; i < childEntries.size(); ++i) {
        if (used[i]) continue;
        auto entry = childEntries[i];
        float enlargeLeft  = enlargement(left->getMBR(), entry.first);
        float enlargeRight = enlargement(right->getMBR(), entry.first);

        if (enlargeLeft < enlargeRight || right->childEntries.empty()) {
            left->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = left;
        } else if (enlargeRight < enlargeLeft || left->childEntries.empty()) {
            right->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = right;
        } else if (left->getMBR().volume() <= right->getMBR().volume()) {
            left->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = left;
        } else {
            right->childEntries.push_back(entry);
            if (entry.second) entry.second->parent = right;
        }
    }

    left->updateMBR();
    right->updateMBR();
    childEntries.clear();

    // Avoid returning empty nodes
    return { left->isEmpty() ? nullptr : left,
             right->isEmpty() ? nullptr : right };
}
*/



/*
void RTreeNode::insert(const BoundingBox3D& box, std::shared_ptr<RTreeNode> child) {
    childEntries.emplace_back(box, child);
    child->parent = shared_from_this();
    markDirty();
}*/


/*
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitLeaf() {
    float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest();
    float minY = minX, maxY = maxX;

    for (const auto& [box, _] : leafEntries) {
        minX = std::min(minX, box.getMinX());
        maxX = std::max(maxX, box.getMaxX());
        minY = std::min(minY, box.getMinY());
        maxY = std::max(maxY, box.getMaxY());
    }

    float xSpan = maxX - minX;
    float ySpan = maxY - minY;

    if (xSpan >= ySpan) {
        std::sort(leafEntries.begin(), leafEntries.end(), [](const auto& a, const auto& b) {
            return a.first.getMinX() < b.first.getMinX();
        });
    } else {
        std::sort(leafEntries.begin(), leafEntries.end(), [](const auto& a, const auto& b) {
            return a.first.getMinY() < b.first.getMinY();
        });
    }

    size_t mid = leafEntries.size() / 2;
    auto left = std::make_shared<RTreeNode>(true, maxEntries);
    auto right = std::make_shared<RTreeNode>(true, maxEntries);
         
    left->leafEntries.reserve(mid);
    right->leafEntries.reserve(leafEntries.size() - mid);
    left->leafEntries.assign(leafEntries.begin(), leafEntries.begin() + mid);
    right->leafEntries.assign(leafEntries.begin() + mid, leafEntries.end());

    left->markDirty();
    right->markDirty();

    return {left, right};
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTreeNode::splitInternal() {
    float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest();
    float minY = minX, maxY = maxX;

    for (const auto& [box, _] : childEntries) {
        minX = std::min(minX, box.getMinX());
        maxX = std::max(maxX, box.getMaxX());
        minY = std::min(minY, box.getMinY());
        maxY = std::max(maxY, box.getMaxY());
    }

    float xSpan = maxX - minX;
    float ySpan = maxY - minY;

    if (xSpan >= ySpan) {
        std::sort(childEntries.begin(), childEntries.end(), [](const auto& a, const auto& b) {
            return a.first.getMinX() < b.first.getMinX();
        });
    } else {
        std::sort(childEntries.begin(), childEntries.end(), [](const auto& a, const auto& b) {
            return a.first.getMinY() < b.first.getMinY();
        });
    }

    size_t mid = childEntries.size() / 2;
    auto left = std::make_shared<RTreeNode>(false, maxEntries);
    auto right = std::make_shared<RTreeNode>(false, maxEntries);


    left->childEntries.reserve(mid);
    right->childEntries.reserve(childEntries.size() - mid);
    left->childEntries.assign(childEntries.begin(), childEntries.begin() + mid);
    right->childEntries.assign(childEntries.begin() + mid, childEntries.end());

    for (auto& [_, child] : left->childEntries) child->parent = left;
    for (auto& [_, child] : right->childEntries) child->parent = right;

    left->markDirty();
    right->markDirty();

    return {left, right};
}
*/