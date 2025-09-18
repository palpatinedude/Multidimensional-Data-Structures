/**
 * RTree.h
 * ----------
 * High-level interface for a 3D R-Tree spatial index of trajectories.
 * 
 * Purpose:
 * - Manages a tree of RTreeNode instances.
 * - Provides insertion, deletion, update, range queries, kNN, and similarity search.
 * - Supports bulk loading, JSON import/export, and statistical queries.
 * 
 * Key points:
 * - RTreeNode represents nodes (internal or leaf) storing bounding boxes and trajectories.
 * - RTree delegates recursion and node-level logic to RTreeNode.
 * - This class is the entry point for users interacting with the tree.
 */ 

#ifndef RTREE_H
#define RTREE_H

#include <memory>
#include <vector>
#include <string>
#include "RTreeNode.h"
#include "trajectory.h"
#include "bbox3D.h"

struct TrajectorySummary {
    std::string id;
    BoundingBox3D bbox;           // precomputed bounding box
    float centroidX;              // mean x
    float centroidY;              // mean y
    float centroidT;              // mean timestamp in seconds
    std::shared_ptr<Trajectory> fullTrajectory; // pointer to full trajectory
};

class RTree {
private:
    std::shared_ptr<RTreeNode> root;   // Root node of the tree
    int maxEntries;                    // Maximum entries per node

    // ---------------- Stats ----------------
    size_t getTotalEntries() const;  // Count total trajectories
   // int getHeight() const;           // Compute tree height

public:
    // ---------------- Constructors ----------------
    explicit RTree(int maxEntries = 8);

    // ---------------- Data modification ----------------
    void insert(const Trajectory& traj);     // Insert trajectory
    bool remove(const std::string& trajId);  // Remove trajectory by ID
    bool update(const Trajectory& traj);     // Update trajectory (delete + insert if needed)
    void bulkLoad(std::vector<Trajectory>& trajectories); // Build tree using STR bulk-loading

    // ---------------- Helper for faster queries ----------------
    std::vector<TrajectorySummary> computeSummaries(const std::vector<Trajectory>& trajectories); // Precompute summaries for trajectories allowing fast pruning

    // ---------------- Query operations ----------------
    std::vector<Trajectory> rangeQuery(const BoundingBox3D& queryBox) const;  // Spatial range search
   // std::vector<Trajectory> kNearestNeighbors(const Trajectory& query, size_t k) const; // k-NN query
    std::vector<Trajectory> kNearestNeighbors(const Trajectory& query, size_t k, float timeScale = 1e-5f) const;
    std::vector<Trajectory> findSimilar(const Trajectory& query, float maxDistance) const; // Similarity search
    std::vector<Trajectory> getAllLeafTrajectories() const; // Retrieve all trajectories in leaves

    // ---------------- Persistence ----------------
    void exportToJSON(const std::string& filename) const;      // Save to JSON file
    //static std::vector<Trajectory> loadFromJSON(const std::string& filepath); // Load trajectories from JSON
    static std::vector<Trajectory> loadFromParquet(const std::string& filepath); // Load trajectories from Parquet file

    // ---------------- Print Statistics ----------------
    void printStatistics() const;    // Print tree stats

    // ---------------- Getter ----------------
    std::shared_ptr<RTreeNode> getRoot() const { return root; }
    int getHeight() const;           // Compute tree height
};

#endif // RTREE_H