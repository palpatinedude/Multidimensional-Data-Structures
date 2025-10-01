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
    if (bestIndex < 0) {
        throw std::runtime_error("chooseSubtree failed to pick a child , unexpected state.");
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
// ---------------- Insert into internal node ----------------
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::insertIntoInternal(const Trajectory &traj) {
    // ---------------- Safety Checks ----------------
    if (childEntries.empty()) {
       // std::cerr << "[insertIntoInternal] ERROR: Internal node has no children!\n";
        return {nullptr, nullptr};
    }

    // Choose the best child for insertion
    int bestChildIndex = chooseSubtree(traj.getBoundingBox());
    if (bestChildIndex < 0 || bestChildIndex >= static_cast<int>(childEntries.size())) {
    //    std::cerr << "[insertIntoInternal] ERROR: chooseSubtree returned invalid index "
       //           << bestChildIndex << "\n";
        return {nullptr, nullptr};
    }

    auto childNode = childEntries[bestChildIndex].second;
    if (!childNode) {
      //  std::cerr << "[insertIntoInternal] ERROR: Null child at index " << bestChildIndex << "\n";
        return {nullptr, nullptr};
    }

    //std::cout << "[insertIntoInternal] Inserting into child " << bestChildIndex
             // << " with MBR: " << childNode->getMBR() << "\n";

    // ---------------- Recursive Insert ----------------
    auto [splitLeft, splitRight] = childNode->insertRecursive(traj);

    // ---------------- Handle Child Split ----------------
    if (splitLeft && splitRight) {
       // std::cout << "[insertIntoInternal] Child split detected. Updating internal node.\n";

        // Remove old child
        childEntries.erase(childEntries.begin() + bestChildIndex);

        // Insert the two new split nodes
        insertChild(splitLeft->getMBR(), splitLeft);
        insertChild(splitRight->getMBR(), splitRight);

       // std::cout << "[insertIntoInternal] Internal node now has " << childEntries.size()
        //          << " children after split.\n";

        // If this internal node is overfull, split it
        if (static_cast<int>(childEntries.size()) > maxEntries) {
          //  std::cout << "[insertIntoInternal] Internal node overfull. Splitting internal node.\n";
            return splitInternal(); // Returns {left, right} to parent
        }
    }

    // ---------------- Update this node's MBR ----------------
    updateMBR();

    // No split at this level
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

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> 
RTreeNode::splitInternal() {
    if (childEntries.empty()) {
        std::cerr << "[splitInternal] ERROR: Cannot split internal node with no children!\n";
        return {nullptr, nullptr};
    }

    // Create two new internal nodes
    auto leftNode  = std::make_shared<RTreeNode>(false, maxEntries);
    auto rightNode = std::make_shared<RTreeNode>(false, maxEntries);

    // Split entries into left/right using quadratic split
    quadraticSplitEntries(childEntries, leftNode, rightNode, maxEntries);

    // Clear current node's child entries (original node will be replaced)
    childEntries.clear();

    // Update MBRs for left/right nodes
    leftNode->updateMBR();
    rightNode->updateMBR();

    // Debug info
  //  std::cout << "[splitInternal] Split completed. Left node children: " 
    //          << leftNode->getChildEntries().size() 
    //          << ", Right node children: " 
     //         << rightNode->getChildEntries().size() << "\n";

    return {leftNode, rightNode};
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
    BoundingBox3D queryBox = query.getBoundingBox(); // use precomputed bounding box

    // Prune node if minimum distance to queryBox exceeds threshold
    if (!isLeaf) {
        float minDistSq = getMBR().distanceSquaredTo(queryBox);
        if (minDistSq > maxDistance * maxDistance) return; // cannot contain similar trajectories
    } else {
        // For leaf nodes, still check MBR intersection for fast prune
        if (!getMBR().intersects(queryBox) && maxDistance > 0.0f) return;
    }

    if (isLeaf) {
        // Check each trajectory in the leaf
        for (const auto& [_, trajPtr] : leafEntries) {
            if (!trajPtr) continue;

            // Fast approximate check using centroids / bounding boxes
            float approxDist = query.approximateDistance(*trajPtr, 1e-5f);
            if (approxDist <= maxDistance) {
                // Optional: recompute exact spatio-temporal similarity
                if (query.similarityTo(*trajPtr) <= maxDistance) {
                    results.push_back(*trajPtr);
                }
            }
        }
    } else {
        // Recurse into children
        for (const auto& [childBox, child] : childEntries) {
            float minDistSq = childBox.distanceSquaredTo(queryBox);
            if (minDistSq <= maxDistance * maxDistance) {
                child->findSimilar(query, maxDistance, results);
            }
        }
    }
}

std::vector<Trajectory> RTreeNode::kNearestNeighbors(
    const Trajectory& query, 
    size_t k, 
    float timeScale, 
    size_t candidateMultiplier) const
{
    using ResultPair = std::pair<float, std::shared_ptr<Trajectory>>; // distance^2, trajectory pointer

    // Max-heap: largest distance on top
    auto cmpMax = [](const ResultPair& a, const ResultPair& b) { return a.first < b.first; };
    std::priority_queue<ResultPair, std::vector<ResultPair>, decltype(cmpMax)> knn(cmpMax);

    // Min-heap for RTree node traversal
    using HeapEntry = std::pair<float, std::shared_ptr<RTreeNode>>;
    auto cmpMin = [](const HeapEntry& a, const HeapEntry& b) { return a.first > b.first; };
    std::priority_queue<HeapEntry, std::vector<HeapEntry>, decltype(cmpMin)> pq(cmpMin);

    // Start from this node
    pq.push({0.0f, std::const_pointer_cast<RTreeNode>(shared_from_this())});

    size_t kCandidates = std::max<size_t>(k * candidateMultiplier, k);

    auto getFarthestDistSq = [&]() -> float {
        return knn.empty() ? std::numeric_limits<float>::infinity() : knn.top().first;
    };

    const BoundingBox3D& queryBox = query.getBoundingBox();

    while (!pq.empty()) {
        auto [nodeDistSq, node] = pq.top(); pq.pop();

        // prune entire branch
        if (nodeDistSq > getFarthestDistSq()) break;

        if (node->isLeaf) {
            // Leaf nodes: check trajectories
            for (const auto& [box, trajPtr] : node->leafEntries) {
                if (!trajPtr) continue;

                // Step 1: approximate distance (fast)
                float approxDistSq = query.approximateDistance(*trajPtr, timeScale);
                if (knn.size() < kCandidates || approxDistSq < getFarthestDistSq()) {
                    // Step 2: exact spatio-temporal distance (expensive, only if promising)
                    float exactDistSq = query.spatioTemporalDistanceTo(*trajPtr, timeScale);

                    if (knn.size() < kCandidates || exactDistSq < getFarthestDistSq()) {
                        knn.push({exactDistSq, trajPtr});
                        if (knn.size() > kCandidates) knn.pop();
                    }
                }
            }
        } else {
            // Internal nodes: prune by MBR distance
            for (const auto& [childBox, child] : node->childEntries) {
                float minDistSq = queryBox.distanceSquaredTo(childBox);
                if (minDistSq <= getFarthestDistSq()) {
                    pq.push({minDistSq, child});
                }
            }
        }
    }

    // Collect top-k candidates
    std::vector<ResultPair> candidates;
    while (!knn.empty()) { candidates.push_back(knn.top()); knn.pop(); }
    std::sort(candidates.begin(), candidates.end(),
              [](const ResultPair& a, const ResultPair& b){ return a.first < b.first; });

    // Filter duplicates & exclude query itself
    std::vector<Trajectory> results;
    std::unordered_set<std::string> seen;
    for (auto& [distSq, trajPtr] : candidates) {
        if (!trajPtr) continue;
        const std::string& tid = trajPtr->getId();
        if (tid == query.getId()) continue;
        if (seen.insert(tid).second) {
            results.push_back(*trajPtr);
            if (results.size() >= k) break;
        }
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

void RTreeNode::condenseTree() {
    auto p = parent.lock(); // get parent node
    if (!p) return; // stop if this is the root (no parent)

    std::vector<std::shared_ptr<RTreeNode>> toReinsert; // store children that need reinsertion

    if (isLeaf) {
        // If leaf node has no entries, remove it from its parent
        if (leafEntries.empty()) removeFromParent();
    } else {
        // Internal node
        if (childEntries.empty()) {
            // No children, remove from parent
            removeFromParent();
        } else if ((int)childEntries.size() < (maxEntries + 1)/2) {
            // Node underflows (less than minimum allowed entries)
            // Collect children to reinsert later
            for (auto& [_, child] : childEntries) toReinsert.push_back(child);
            childEntries.clear(); // remove all children
            removeFromParent();   // remove this node from its parent
        }
    }

    if (p) {
        // Recursively condense up the tree
        p->condenseTree();

        // Reinsert orphaned children into the parent
        for (auto& child : toReinsert) 
            p->insertChild(child->getMBR(), child);
    }
}


// ---------------- Operators & Serialization ----------------
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



// Getters 
const std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& RTreeNode::getLeafEntries() const{
    return leafEntries;
}

const std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>& RTreeNode::getChildEntries() const{
    return childEntries;
}







