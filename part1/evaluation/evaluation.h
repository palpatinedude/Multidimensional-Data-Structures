// ============================================================================
// evaluation.h
//
// Provides tools to evaluate RTree performance against linear scan for trajectory queries.
// - Supports range queries, k-nearest neighbors (kNN), and similarity queries.
// - Measures query time, result count, and uniqueness.
// - Saves individual query results and overall summaries to CSV files.
// ============================================================================
#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include "../api/include/trajectory.h"
#include "../api/include/RTree.h"
#include "../api/include/bbox3D.h"

// Structure to store query statistics
struct QueryStats {
    std::string type;
    std::string city;
    std::string trajId;
    std::string startTime;
    std::string endTime;
    size_t k = 0;
    float threshold = 0.0f;

    size_t rtreeCount = 0;
    size_t rtreeUniqueVehicles = 0;
    double rtreeTime = 0.0;
    size_t linearCount = 0;
    size_t linearUniqueVehicles = 0;
    double linearTime = 0.0;
};

struct QueryResult {
    std::string trajId;
    float distance = 0.0f;       // For kNN or similarity
    float similarity = 0.0f;     // Similarity score
    size_t numPoints = 0;
};

class Evaluation {
private:
    RTree& rtree;                              
    const std::vector<Trajectory> trajectories;     
    const std::vector<Trajectory> trajectoriesCopy; 
    std::string folder;                         

    void saveQueryResults(int queryIndex, const std::string& queryType,
                          const std::vector<QueryResult>& rtreeResults,
                          const std::vector<QueryResult>& linearResults);

    void saveQueryTrajectoriesForPlot(int queryIndex, 
                                      const std::string& queryType,
                                      const Trajectory* queryTraj,
                                      const std::vector<Trajectory>& results);

    std::vector<Trajectory> linearScan(
        const std::function<bool(const Trajectory&)>& predicate,
        const Trajectory* exclude,
        size_t& outCount,
        size_t& outUnique,
        size_t maxCount = 0,
        const std::function<float(const Trajectory&)>& distanceFunc = nullptr
    );

    std::vector<Trajectory> filterUniqueTrajectories(const std::vector<Trajectory>& input,
                                                     const Trajectory* exclude = nullptr,
                                                     size_t maxCount = 0);

    const Trajectory* findTrajectoryById(const std::string& trajId);                                                 

public:
    Evaluation(RTree& tree,
               const std::vector<Trajectory> trajs,
               const std::vector<Trajectory> trajsCopy,
               const std::string& resultFolder = "results");

    QueryStats runRangeQuery(const std::string& city,
                             const std::string& startTime,
                             const std::string& endTime,
                             int queryIndex);

    QueryStats runKNNQuery(const std::string& trajId, size_t k, int queryIndex);

    QueryStats runSimilarityQuery(const std::string& trajId, float threshold, int queryIndex);

    void saveSummary(const std::vector<QueryStats>& statsList);

    const std::vector<Trajectory>& getTrajectories() const { return trajectories; }
};

#endif // EVALUATION_H


/*
#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include "../api/include/trajectory.h"
#include "../api/include/RTree.h"
#include "../api/include/bbox3D.h"

// Structure to store query statistics
struct QueryStats {
    std::string type;
    std::string city;
    std::string trajId;
    std::string startTime;
    std::string endTime;
    size_t k = 0;
    float threshold = 0.0f;

    size_t rtreeCount = 0;
    size_t rtreeUniqueVehicles = 0;
    double rtreeTime = 0.0;
    size_t linearCount = 0;
    size_t linearUniqueVehicles = 0;
    double linearTime = 0.0;
};

class Evaluation {
private:
    RTree& rtree;                              // RTree index for queries
    const std::vector<Trajectory> trajectories;     // Used for RTree queries
    const std::vector<Trajectory> trajectoriesCopy; // Used for linear scan
    std::string folder;                         // Output folder for results

    void saveQueryResults(int queryIndex, const std::string& queryType,
                          const std::vector<Trajectory>& rtreeResults,
                          const std::vector<Trajectory>& linearResults); // Save query results 

    void saveQueryResultsForPlot(int queryIndex, 
                                         const std::string& queryType,
                                         const Trajectory* queryTraj,
                                         const std::vector<Trajectory>& results);                    


    std::vector<Trajectory> linearScan(
        const std::function<bool(const Trajectory&)>& predicate,
        const Trajectory* exclude,
        size_t& outCount,
        size_t& outUnique,
        size_t maxCount = 0,
        const std::function<float(const Trajectory&)>& distanceFunc = nullptr
    ); // Unified linear scan for all queries for comparison

    std::vector<Trajectory> filterUniqueTrajectories(const std::vector<Trajectory>& input,
                                                     const Trajectory* exclude = nullptr,
                                                     size_t maxCount = 0); // Keep only unique trajectories 

    const Trajectory* findTrajectoryById(const std::string& trajId); // Find trajectory by ID                                                 

public:
    // ---------------- Constructor ----------------
    Evaluation(RTree& tree,
               const std::vector<Trajectory> trajs,
               const std::vector<Trajectory> trajsCopy,
               const std::string& resultFolder = "results");

    // ---------------- Query methods ----------------
    QueryStats runRangeQuery(const std::string& city,
                             const std::string& startTime,
                             const std::string& endTime,
                             int queryIndex);

    QueryStats runKNNQuery(const std::string& trajId, size_t k, int queryIndex);

    QueryStats runSimilarityQuery(const std::string& trajId, float threshold, int queryIndex);

    // ---------------- Result processing ----------------
    void saveSummary(const std::vector<QueryStats>& statsList);

    // ---------------- Getters ----------------
    const std::vector<Trajectory>& getTrajectories() const { return trajectories; }

   

};

#endif // EVALUATION_H
*/