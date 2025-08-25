
/*
#include "evaluation.h"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <unordered_set>

Evaluation::Evaluation(RTree& tree,
                       const std::vector<Trajectory>& trajs,
                       const std::vector<Trajectory>& trajsCopy,
                       const std::string& resultFolder)
    : rtree(tree), trajectories(trajs), trajectoriesCopy(trajsCopy), folder(resultFolder) {}

// ===== Helper: find trajectory by ID in RTree dataset =====
const Trajectory* Evaluation::findTrajectoryById(const std::string& trajId) {
    for (auto& t : trajectories) {
        if (t.getId() == trajId) return &t;
    }
    return nullptr;
}

// ===== Helper: save query results to CSV =====
void Evaluation::saveQueryResults(int queryIndex, const std::string& queryType,
                                  const std::vector<Trajectory>& rtreeResults,
                                  const std::vector<Trajectory>& linearResults) {
    std::ofstream queryOut(folder + "/query_" + std::to_string(queryIndex) + "_" + queryType + ".csv");
    if (!queryOut) return;
    queryOut << "Method,TrajectoryID\n";
    for (auto& traj : rtreeResults) queryOut << "RTree," << traj.getId() << "\n";
    for (auto& traj : linearResults) queryOut << "Linear," << traj.getId() << "\n";
}

// ===== Linear scan helper =====
std::vector<Trajectory> Evaluation::linearScan(
    const std::function<bool(const Trajectory&)>& predicate,
    size_t& outCount,
    size_t& outUnique) 
{
    std::vector<Trajectory> results;
    std::unordered_set<std::string> vehicleIds;

    for (auto& t : trajectoriesCopy) {
        if (predicate(t)) {
            results.push_back(t);
            vehicleIds.insert(t.getId());
        }
    }

    outCount = results.size();
    outUnique = vehicleIds.size();
    return results;
}

// ===== Range query =====
QueryStats Evaluation::runRangeQuery(const std::string& city,
                                     const std::string& startTime,
                                     const std::string& endTime,
                                     int queryIndex) {
    QueryStats qs;
    qs.type = "rangeQuery";
    qs.city = city;
    qs.startTime = startTime;
    qs.endTime = endTime;

    float minX, minY, maxX, maxY;
    if (city == "Philadelphia") { minX=-75.28; maxX=-75.16; minY=39.87; maxY=40.00; }
    else if (city == "Atlanta") { minX=-84.45; maxX=-84.35; minY=33.70; maxY=33.85; }
    else if (city == "Memphis") { minX=-90.10; maxX=-89.90; minY=35.05; maxY=35.20; }
    else { minX=-75.28; maxX=-75.16; minY=39.87; maxY=40.00; }

    BoundingBox3D queryBox(minX, minY, startTime, maxX, maxY, endTime);

    // --- RTree query ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.rangeQuery(queryBox);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    std::unordered_set<std::string> vehicleIds;
    for (auto& t : rtreeResults) vehicleIds.insert(t.getId());
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = vehicleIds.size();

    // --- Linear scan ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<Trajectory> linearResults = linearScan(
        [&](const Trajectory& t){ return t.getBoundingBox().intersects(queryBox); },
        qs.linearCount, qs.linearUniqueVehicles
    );
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}

// ===== KNN query =====

QueryStats Evaluation::runKNNQuery(const std::string& trajId, size_t k, int queryIndex) {
    QueryStats qs;
    qs.type = "kNearestNeighbors";
    qs.trajId = trajId;
    qs.k = k;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    // --- RTree KNN ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.kNearestNeighbors(*target, k);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    std::unordered_set<std::string> vehicleIds;
    for (auto& t : rtreeResults) vehicleIds.insert(t.getId());
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = vehicleIds.size();

    // --- Linear scan KNN ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<float, Trajectory>> distances;
    for (auto& t : trajectoriesCopy) {
        if (t.getId() != target->getId()) { // skip self
            distances.push_back({target->similarityTo(t), t});
        }
    }
    std::sort(distances.begin(), distances.end(),
              [](auto &a, auto &b){ return a.first < b.first; });

    std::vector<Trajectory> linearResults;
    for (size_t i=0; i<std::min(k, distances.size()); ++i)
        linearResults.push_back(distances[i].second);
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    vehicleIds.clear();
    for (auto& t : linearResults) vehicleIds.insert(t.getId());
    qs.linearCount = linearResults.size();
    qs.linearUniqueVehicles = vehicleIds.size();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}
*/
/*
QueryStats Evaluation::runKNNQuery(const std::string& trajId, size_t k, int queryIndex) {
    QueryStats qs;
    qs.type = "kNearestNeighbors";
    qs.trajId = trajId;
    qs.k = k;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    // --- RTree KNN ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.kNearestNeighbors(*target, k, 1e-5f); // use spatio-temporal distance
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    std::unordered_set<std::string> vehicleIds;
    std::vector<Trajectory> filteredRTreeResults;
    for (auto& t : rtreeResults) {
        std::string vid = t.getId();
        if (vid != target->getId() && vehicleIds.insert(vid).second) {
            filteredRTreeResults.push_back(t);
            if (filteredRTreeResults.size() >= k) break;
        }
    }
    qs.rtreeCount = filteredRTreeResults.size();
    qs.rtreeUniqueVehicles = vehicleIds.size();

    // --- Linear scan KNN ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<float, Trajectory>> distances;
    for (auto& t : trajectoriesCopy) {
        std::string vid = t.getId();
        if (vid != target->getId()) { // skip self
            float dist = target->spatioTemporalDistanceTo(t, 1e-5f);
            distances.push_back({dist, t});
        }
    }

    std::sort(distances.begin(), distances.end(),
              [](auto &a, auto &b){ return a.first < b.first; });

    std::vector<Trajectory> linearResults;
    vehicleIds.clear();
    for (auto& [dist, t] : distances) {
        std::string vid = t.getId();
        if (vehicleIds.insert(vid).second) {
            linearResults.push_back(t);
            if (linearResults.size() >= k) break;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    qs.linearCount = linearResults.size();
    qs.linearUniqueVehicles = vehicleIds.size();

    saveQueryResults(queryIndex, qs.type, filteredRTreeResults, linearResults);
    return qs;
}

// ===== Similarity query =====
QueryStats Evaluation::runSimilarityQuery(const std::string& trajId, float threshold, int queryIndex) {
    QueryStats qs;
    qs.type = "findSimilar";
    qs.trajId = trajId;
    qs.threshold = threshold;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    // --- RTree Similarity ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.findSimilar(*target, threshold);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    std::unordered_set<std::string> vehicleIds;
    for (auto& t : rtreeResults) vehicleIds.insert(t.getId());
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = vehicleIds.size();

    // --- Linear scan Similarity ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<Trajectory> linearResults = linearScan(
        [&](const Trajectory& t){ return target->similarityTo(t) <= threshold; },
        qs.linearCount, qs.linearUniqueVehicles
    );
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}

// ===== Save summary =====
void Evaluation::saveSummary(const std::vector<QueryStats>& statsList) {
    std::ofstream summaryOut(folder + "/query_summary.csv");
    if (!summaryOut) return;

    summaryOut << "QueryType,City,TrajectoryID,StartTime,EndTime,k,Threshold,"
                  "RTreeCount,RTreeVehicles,RTreeTime(s),LinearCount,LinearVehicles,LinearTime(s)\n";

    for (auto& s : statsList) {
        summaryOut << std::fixed << std::setprecision(6)
                   << s.type << "," << s.city << "," << s.trajId << "," << s.startTime << ","
                   << s.endTime << "," << s.k << "," << s.threshold << ","
                   << s.rtreeCount << "," << s.rtreeUniqueVehicles << "," << s.rtreeTime << ","
                   << s.linearCount << "," << s.linearUniqueVehicles << "," << s.linearTime << "\n";
    }
}
*/

#include "evaluation.h"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <unordered_set>
#include <functional>

Evaluation::Evaluation(RTree& tree,
                       const std::vector<Trajectory>& trajs,
                       const std::vector<Trajectory>& trajsCopy,
                       const std::string& resultFolder)
    : rtree(tree), trajectories(trajs), trajectoriesCopy(trajsCopy), folder(resultFolder) {}

// ===== Find trajectory by ID =====
const Trajectory* Evaluation::findTrajectoryById(const std::string& trajId) {
    for (auto& t : trajectories) {
        if (t.getId() == trajId) return &t;
        else std::cout << "[Mismatch] '" << t.getId() << "' vs '" << trajId << "'\n";
    }
    return nullptr;
}

// ===== Save query results to CSV =====
void Evaluation::saveQueryResults(int queryIndex, const std::string& queryType,
                                  const std::vector<Trajectory>& rtreeResults,
                                  const std::vector<Trajectory>& linearResults) {
    std::ofstream queryOut(folder + "/query_" + std::to_string(queryIndex) + "_" + queryType + ".csv");
    if (!queryOut) return;

    queryOut << "Method,TrajectoryID\n";
    for (auto& traj : rtreeResults) queryOut << "RTree," << traj.getId() << "\n";
    for (auto& traj : linearResults) queryOut << "Linear," << traj.getId() << "\n";
}

// ===== Remove duplicates =====
std::vector<Trajectory> Evaluation::filterUniqueTrajectories(const std::vector<Trajectory>& input,
                                                             const Trajectory* exclude,
                                                             size_t maxCount) {
    std::vector<Trajectory> results;
    std::unordered_set<std::string> seenIds;

    for (auto& t : input) {
        std::string tid = t.getId();
        if ((exclude && tid == exclude->getId()) || !seenIds.insert(tid).second)
            continue;

        results.push_back(t);
        if (maxCount > 0 && results.size() >= maxCount)
            break;
    }
    return results;
}

// ===== Linear scan helper =====
std::vector<Trajectory> Evaluation::linearScan(const std::function<bool(const Trajectory&)>& predicate,
                                               const Trajectory* exclude,
                                               size_t& outCount,
                                               size_t& outUnique,
                                               size_t maxCount) {
    std::vector<Trajectory> candidates;
    for (auto& t : trajectoriesCopy) {
        if (predicate(t))
            candidates.push_back(t);
    }

    std::vector<Trajectory> uniqueTrajs = filterUniqueTrajectories(candidates, exclude, maxCount);
    outCount = candidates.size();
    outUnique = uniqueTrajs.size();
    return uniqueTrajs;
}

// ===== Range query =====
QueryStats Evaluation::runRangeQuery(const std::string& city,
                                     const std::string& startTime,
                                     const std::string& endTime,
                                     int queryIndex) {
    QueryStats qs;
    qs.type = "rangeQuery";
    qs.city = city;
    qs.startTime = startTime;
    qs.endTime = endTime;

    float minX, minY, maxX, maxY;
    if (city == "Philadelphia") { minX=-75.28; maxX=-75.16; minY=39.87; maxY=40.00; }
    else if (city == "Atlanta") { minX=-84.45; maxX=-84.35; minY=33.70; maxY=33.85; }
    else if (city == "Memphis") { minX=-90.10; maxX=-89.90; minY=35.05; maxY=35.20; }
    else { minX=-75.28; maxX=-75.16; minY=39.87; maxY=40.00; }

    BoundingBox3D queryBox(minX, minY, startTime, maxX, maxY, endTime);

    // --- RTree query ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.rangeQuery(queryBox);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    // --- Linear scan ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<Trajectory> linearResults = linearScan(
        [&](const Trajectory& t){ return t.getBoundingBox().intersects(queryBox); },
        nullptr, qs.linearCount, qs.linearUniqueVehicles
    );
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}

// ===== KNN query =====
QueryStats Evaluation::runKNNQuery(const std::string& trajId, size_t k, int queryIndex) {
    std::cout<<"in the begging in the runknnquery" <<std::endl;
    QueryStats qs;
    qs.type = "kNearestNeighbors";
    qs.trajId = trajId;
    qs.k = k;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    float timeScale = 1e-5f;

    std::cout<<"after the findtrajectory"<<std::endl;

    // --- RTree KNN ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.kNearestNeighbors(*target, k, timeScale);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults, target, k);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    // --- Linear scan KNN ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<float, Trajectory>> distances;
    for (auto& t : trajectoriesCopy) {
        if (t.getId() != target->getId()) {
            distances.push_back({target->spatioTemporalDistanceTo(t, timeScale), t});
        }
    }
    std::sort(distances.begin(), distances.end(),
              [](auto &a, auto &b){ return a.first < b.first; });

    std::vector<Trajectory> linearCandidates;
    for (auto& [dist, t] : distances) linearCandidates.push_back(t);

    std::vector<Trajectory> linearResults = filterUniqueTrajectories(linearCandidates, target, k);
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    qs.linearCount = linearResults.size();
    qs.linearUniqueVehicles = linearResults.size();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}

// ===== Similarity query =====
QueryStats Evaluation::runSimilarityQuery(const std::string& trajId, float threshold, int queryIndex) {
    QueryStats qs;
    qs.type = "findSimilar";
    qs.trajId = trajId;
    qs.threshold = threshold;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    // --- RTree similarity ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.findSimilar(*target, threshold);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults, target);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    // --- Linear scan similarity ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<Trajectory> linearResults = linearScan(
        [&](const Trajectory& t){ return target->similarityTo(t) <= threshold; },
        target, qs.linearCount, qs.linearUniqueVehicles
    );
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}

// ===== Save summary =====
void Evaluation::saveSummary(const std::vector<QueryStats>& statsList) {
    std::ofstream summaryOut(folder + "/query_summary.csv");
    if (!summaryOut) return;

    summaryOut << "QueryType,City,TrajectoryID,StartTime,EndTime,k,Threshold,"
                  "RTreeCount,RTreeTrajIDs,RTreeTime(s),LinearCount,LinearTrajIDs,LinearTime(s)\n";

    for (auto& s : statsList) {
        summaryOut << std::fixed << std::setprecision(6)
                   << s.type << "," << s.city << "," << s.trajId << "," << s.startTime << ","
                   << s.endTime << "," << s.k << "," << s.threshold << ","
                   << s.rtreeCount << "," << s.rtreeUniqueVehicles << "," << s.rtreeTime << ","
                   << s.linearCount << "," << s.linearUniqueVehicles << "," << s.linearTime << "\n";
    }
}
