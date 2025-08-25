#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include "../api/include/trajectory.h"
#include "../api/include/RTree.h"
#include "../api/include/bbox3D.h"

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
    RTree& rtree;
    const std::vector<Trajectory>& trajectories;     // used by RTree
    const std::vector<Trajectory>& trajectoriesCopy; // used by Linear Scan
    std::string folder;

    // Save query results to CSV
    void saveQueryResults(int queryIndex, const std::string& queryType,
                          const std::vector<Trajectory>& rtreeResults,
                          const std::vector<Trajectory>& linearResults);

    // Find trajectory by ID
    const Trajectory* findTrajectoryById(const std::string& trajId);

    // Linear scan helper: returns matching trajectories, fills count/unique vehicles
    std::vector<Trajectory> linearScan(const std::function<bool(const Trajectory&)>& predicate,
                                       const Trajectory* exclude,
                                       size_t& outCount,
                                       size_t& outUnique,
                                       size_t maxCount = 0);

public:
    Evaluation(RTree& tree,
               const std::vector<Trajectory>& trajs,
               const std::vector<Trajectory>& trajsCopy,
               const std::string& resultFolder = "results");

    QueryStats runRangeQuery(const std::string& city,
                             const std::string& startTime,
                             const std::string& endTime,
                             int queryIndex);

    QueryStats runKNNQuery(const std::string& trajId, size_t k, int queryIndex);

    QueryStats runSimilarityQuery(const std::string& trajId, float threshold, int queryIndex);

    void saveSummary(const std::vector<QueryStats>& statsList);

    // Remove duplicates and optionally exclude a trajectory
    std::vector<Trajectory> filterUniqueTrajectories(const std::vector<Trajectory>& input,
                                                     const Trajectory* exclude = nullptr,
                                                     size_t maxCount = 0);
};

#endif // EVALUATION_H
