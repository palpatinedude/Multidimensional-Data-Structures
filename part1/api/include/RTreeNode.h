/*
 * RTreeNode.h
 * ------------
 * Defines a node in a spatiotemporal R-Tree and is the building block of the RTree structure .
 *
 * Each node stores:
 *   - Minimum Bounding Rectangle (MBR) as BoundingBox3D
 *   - Child nodes (internal node) OR trajectory entries (leaf node)
 *   - A weak pointer to its parent
 *
 * Responsibilities:
 *   - Insertion with splitting
 *   - Range queries and similarity searches
 *   - Deletion and updates
 *   - k-Nearest Neighbor search
 *   - Lazy MBR caching
 *  -  Tree structure maintenance (condensation, parent-child relationships)
 */

#ifndef RTREE_NODE_H
#define RTREE_NODE_H

#include "../include/bbox3D.h"
#include "../include/trajectory.h"
#include <memory>
#include <vector>
#include <utility>
#include <string>

class RTreeNode : public std::enable_shared_from_this<RTreeNode> {
private:
    bool isLeaf;                       // True if node is a leaf
    int maxEntries;                     // Maximum entries before splitting
    mutable BoundingBox3D mbr;         // Node's Minimum Bounding Rectangle
    mutable bool mbr_dirty;            // True if MBR needs recomputation

    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> leafEntries; // Leaf node trajectories
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>> childEntries; // Internal node children

    std::weak_ptr<RTreeNode> parent;   // Pointer to parent node

    // ---------------- Internal helper functions ----------------
    void markDirty();                   // Marks MBR as dirty and propagates up
    int chooseSubtree(const BoundingBox3D& bbox) const; // Pick the best child for insertion

    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitLeaf();     // Split full leaf
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitInternal(); // Split full internal

    void condenseTree();               // Adjust tree after deletion
    void removeFromParent();           // Remove this node from its parent

    // Recursive insertion helpers
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> insertIntoLeaf(const Trajectory& traj); // Insert into leaf node
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> insertIntoInternal(const Trajectory& traj); // Insert into internal node

public:
    // ---------------- Constructors ----------------
    RTreeNode(bool isLeaf, int maxEntries);

    // ---------------- Node info ----------------
    bool isLeafNode() const;           // Returns true if leaf
    bool isEmpty() const;              // Returns true if node has no entries
    BoundingBox3D getMBR() const;      // Returns current MBR
    bool needsSplit() const;           // Returns true if node is overfull

    // ---------------- Insertion ----------------
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> insertRecursive(const Trajectory& traj);

    // ---------------- Queries ----------------
    void rangeQuery(const BoundingBox3D& queryBox, std::vector<Trajectory>& results) const;
    void findSimilar(const Trajectory& query, float threshold, std::vector<Trajectory>& results) const;
    std::vector<Trajectory> kNearestNeighbors(const Trajectory& query, size_t k, float timeScale, size_t candidateMultiplier = 50) const;

    // ---------------- Modification ----------------
    bool deleteTrajectory(const std::string& trajId);
    bool updateTrajectory(const Trajectory& traj);

    // ---------------- Accessors ----------------
    const std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& getLeafEntries() const;
    const std::vector<std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>& getChildEntries() const;


    // Update and manage the node's MBR
    void updateMBR() const; // Recompute MBR based on current entries or children
    void recomputeMBRs();  // Recursively recompute MBRs for all descendants

    // Insert entries into the node
    void insertChild(const BoundingBox3D& bbox, std::shared_ptr<RTreeNode> child); // Add a child node (internal)
    void insertLeaf(const BoundingBox3D& bbox, std::shared_ptr<Trajectory> traj);  // Add a trajectory (leaf)

    // Utility functions
    float enlargement(const BoundingBox3D& current, const BoundingBox3D& toAdd) const; // MBR growth if a new entry is added

    // ---------------- Serialization ----------------
    json to_json() const;

    // ---------------- Comparison ----------------
    bool operator==(const RTreeNode& other) const;
    bool operator!=(const RTreeNode& other) const;
};

#endif // RTREE_NODE_H

















/*
    // Split Helpers
    template <typename EntryType>
    static std::pair<int,int> pickSeeds(const std::vector<EntryType>& entries); // Pick two seeds for split

    template <typename EntryType>
    static int pickNextEntry(std::shared_ptr<RTreeNode> left, 
                             std::shared_ptr<RTreeNode> right,
                             const std::vector<EntryType>& entries,
                             const std::vector<bool>& assigned,
                             bool& assignToLeft); // Pick next entry to assign

    template <typename EntryType>
    static bool assignForcedEntries(std::shared_ptr<RTreeNode> left,
                                    std::shared_ptr<RTreeNode> right,
                                    const std::vector<EntryType>& entries,
                                    std::vector<bool>& assigned,
                                    int minFill); // Assign entries to nodes if possible

    template <typename EntryType>
    static void assignEntryToNode(std::shared_ptr<RTreeNode> node, const EntryType& entry); // Assign entry to node

    template <typename EntryType>
    static void quadraticSplitEntries(const std::vector<EntryType>& entries,
                                      std::shared_ptr<RTreeNode> leftNode,
                                      std::shared_ptr<RTreeNode> rightNode,
                                      int maxEntries); // Split entries into two nodes using quadratic split

*/
