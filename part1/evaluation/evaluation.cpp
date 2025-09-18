#include "evaluation.h"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <unordered_set>
#include <functional>

// Utility namespace for timestamp parsing 
namespace timeUtil { int parseTimestampToSeconds(const std::string& timestamp); }

// ---------------- Constructor ----------------
Evaluation::Evaluation(RTree& tree,
                       const std::vector<Trajectory> trajs,
                       const std::vector<Trajectory> trajsCopy,
                       const std::string& resultFolder)
    : rtree(tree), trajectories(trajs), trajectoriesCopy(trajsCopy), folder(resultFolder) {}

// ---------------- Find trajectory by ID ----------------
const Trajectory* Evaluation::findTrajectoryById(const std::string& trajId) {
    for (auto& t : trajectories)
        if (t.getId() == trajId) return &t;
    return nullptr;
}

// ---------------- Filter duplicates and optionally exclude a trajectory ----------------
std::vector<Trajectory> Evaluation::filterUniqueTrajectories(
    const std::vector<Trajectory>& input,
    const Trajectory* exclude,
    size_t maxCount
) {
    std::vector<Trajectory> results;
    std::unordered_set<std::string> seenIds;

    for (const auto& t : input) {
        const std::string& tid = t.getId();

        if ((exclude && tid == exclude->getId()) || !seenIds.insert(tid).second)
            continue;

        results.push_back(t);

        if (maxCount > 0 && results.size() >= maxCount)
            break;
    }

    return results;
}

// ---------------- Linear scan utility ----------------
std::vector<Trajectory> Evaluation::linearScan(
    const std::function<bool(const Trajectory&)>& predicate,
    const Trajectory* exclude,
    size_t& outCount,
    size_t& outUnique,
    size_t maxCount,
    const std::function<float(const Trajectory&)>& distanceFunc
) {
    std::vector<std::pair<float, Trajectory>> candidates;

    for (auto& t : trajectoriesCopy) {
        if ((exclude && t.getId() == exclude->getId()) || !predicate(t)) continue;

        float dist = distanceFunc ? distanceFunc(t) : 0.0f;
        candidates.emplace_back(dist, t);
    }

    outCount = candidates.size();

    if (distanceFunc) {
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b){ return a.first < b.first; });
    }

    std::vector<Trajectory> candidateTrajs;
    for (auto& [_, traj] : candidates) candidateTrajs.push_back(traj);

    auto uniqueTrajs = filterUniqueTrajectories(candidateTrajs, exclude, maxCount);
    outUnique = uniqueTrajs.size();

    return uniqueTrajs;
}

// ---------------- Range Query ----------------
QueryStats Evaluation::runRangeQuery(
    const std::string& city,
    const std::string& startTime,
    const std::string& endTime,
    int queryIndex
) {
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

    int64_t tStart = static_cast<int64_t>(timeUtil::parseTimestampToSeconds(startTime));
    int64_t tEnd   = static_cast<int64_t>(timeUtil::parseTimestampToSeconds(endTime));

    BoundingBox3D queryBox(minX, minY, tStart, maxX, maxY, tEnd);

    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.rangeQuery(queryBox);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    start = std::chrono::high_resolution_clock::now();
    auto linearResults = linearScan(
        [&](const Trajectory& t){ return t.getBoundingBox().intersects(queryBox); },
        nullptr, qs.linearCount, qs.linearUniqueVehicles
    );
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}

QueryStats Evaluation::runSimilarityQuery(
    const std::string& trajId,
    float threshold,
    int queryIndex
) {
    QueryStats qs;
    qs.type = "findSimilar";
    qs.trajId = trajId;
    qs.threshold = threshold;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    // --- RTree Similarity using approximate pruning ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.findSimilar(*target, threshold); // optimized version
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults, target);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    // --- Linear scan with approximate pruning ---
    start = std::chrono::high_resolution_clock::now();
    size_t linearCount, linearUnique;
    auto linearResults = linearScan(
        [&](const Trajectory& t){ 
            float approxDist = target->approximateDistance(t, 1e-5f); 
            return approxDist <= threshold; 
        },
        target,
        linearCount,
        linearUnique
    );

    // Compute exact similarity only for filtered candidates
    linearResults.erase(
        std::remove_if(linearResults.begin(), linearResults.end(),
                       [&](const Trajectory& t){ return target->similarityTo(t) > threshold; }),
        linearResults.end()
    );

    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();
    qs.linearCount = linearCount;
    qs.linearUniqueVehicles = linearResults.size();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    auto queryTraj = findTrajectoryById(trajId);
    saveQueryResultsForPlot(queryIndex, "findSimilar", queryTraj, rtreeResults);
    return qs;
}


QueryStats Evaluation::runKNNQuery(
    const std::string& trajId,
    size_t k,
    int queryIndex
) {
    QueryStats qs;
    qs.type = "kNearestNeighbors";
    qs.trajId = trajId;
    qs.k = k;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    // --- RTree kNN ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.kNearestNeighbors(*target, k, 1e-5f);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults, target, k);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    // --- Linear kNN using linearScan with approximate pruning ---
    start = std::chrono::high_resolution_clock::now();

    size_t linearCount, linearUnique;
    auto linearResults = linearScan(
        [&](const Trajectory& t){ return true; },   // include all
        target,
        linearCount,
        linearUnique,
        k,  // <-- limit to k candidates
        [&](const Trajectory& t){
            return target->approximateDistance(t, 1e-5f);
        }
    );

    // Ensure only top-k candidates
    if (linearResults.size() > k) linearResults.resize(k);

    // Compute exact spatio-temporal distances only for these top-k
    for (auto& traj : linearResults) {
        traj.spatioTemporalDistanceTo(*target, 1e-5f);
    }

    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();
    qs.linearCount = linearCount;
    qs.linearUniqueVehicles = linearResults.size();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    auto queryTraj = findTrajectoryById(trajId);
    saveQueryResultsForPlot(queryIndex, "kNN", queryTraj, rtreeResults);
    return qs;
}


void Evaluation::saveQueryResults(int queryIndex, const std::string& queryType,
                                  const std::vector<Trajectory>& rtreeResults,
                                  const std::vector<Trajectory>& linearResults) 
{
    // Save RTree trajectories with points and bounding boxes
    std::ofstream rtreeOut(folder + "/query_" + std::to_string(queryIndex) + "_" + queryType + "_rtree.csv");
    if (rtreeOut) {
        rtreeOut << "TrajectoryID,MinX,MinY,MaxX,MaxY,PointIndex,X,Y,T\n";
        for (const auto& traj : rtreeResults) {
            auto bbox = traj.getBoundingBox();
            const auto& points = traj.getPoints();
            for (size_t i = 0; i < points.size(); ++i) {
                const auto& p = points[i];
                rtreeOut << traj.getId() << ","
                         << bbox.getMinX() << "," << bbox.getMinY() << "," 
                         << bbox.getMaxX() << "," << bbox.getMaxY() << ","
                         << i << "," << p.getX() << "," << p.getY() << "," << p.getT() << "\n";
            }
        }
    }
    std::ofstream linearOut(folder + "/query_" + std::to_string(queryIndex) + "_" + queryType + "_linear.csv");
    if (linearOut) {
        linearOut << "TrajectoryID\n";
        for (auto& traj : linearResults) linearOut << traj.getId() << "\n";
    }
}

void Evaluation::saveQueryResultsForPlot(int queryIndex, 
                                         const std::string& queryType,
                                         const Trajectory* queryTraj,
                                         const std::vector<Trajectory>& results) 
{
    std::ofstream out(folder + "/query_" + std::to_string(queryIndex) + "_" + queryType + "_plot.csv");
    if (!out) return;

    // Header
    out << "TrajectoryID,PointIndex,X,Y,T,Type\n";

    // Save query trajectory
    if (queryTraj) {
        const auto& points = queryTraj->getPoints();
        for (size_t i = 0; i < points.size(); ++i) {
            const auto& p = points[i];
            out << queryTraj->getId() << "," << i << ","
                << p.getX() << "," << p.getY() << "," << p.getT() 
                << ",query\n";
        }
    }

    // Save result trajectories
    for (const auto& traj : results) {
        const auto& points = traj.getPoints();
        for (size_t i = 0; i < points.size(); ++i) {
            const auto& p = points[i];
            out << traj.getId() << "," << i << ","
                << p.getX() << "," << p.getY() << "," << p.getT() 
                << ",result\n";
        }
    }
}


// ---------------- Save summary of all queries ----------------
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



// ---------------- Save query results to CSV ----------------
/*
void Evaluation::saveQueryResults(int queryIndex, const std::string& queryType,
                                  const std::vector<Trajectory>& rtreeResults,
                                  const std::vector<Trajectory>& linearResults) {
    std::ofstream queryOut(folder + "/query_" + std::to_string(queryIndex) + "_" + queryType + ".csv");
    if (!queryOut) return;

    queryOut << "Method,TrajectoryID\n";
    for (auto& traj : rtreeResults) queryOut << "RTree," << traj.getId() << "\n";
    for (auto& traj : linearResults) queryOut << "Linear," << traj.getId() << "\n";
}*/
/*

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

// ===== Unified Linear Scan =====
std::vector<Trajectory> Evaluation::linearScan(
    const std::function<bool(const Trajectory&)>& predicate,
    const Trajectory* exclude,
    size_t& outCount,
    size_t& outUnique,
    size_t maxCount = 0,
    const std::function<float(const Trajectory&)>& distanceFunc = nullptr
) {
    std::vector<std::pair<float, Trajectory>> candidatesWithDist;

    for (auto& t : trajectoriesCopy) {
        if ((exclude && t.getId() == exclude->getId()) || !predicate(t)) continue;
        float dist = distanceFunc ? distanceFunc(t) : 0.0f;
        candidatesWithDist.push_back({dist, t});
    }

    outCount = candidatesWithDist.size();

    // Sort by distance if distanceFunc is provided
    if (distanceFunc) {
        std::sort(candidatesWithDist.begin(), candidatesWithDist.end(),
                  [](const auto& a, const auto& b){ return a.first < b.first; });
    }

    // Extract unique trajectories
    std::vector<Trajectory> uniqueTrajs;
    std::unordered_set<std::string> seenIds;
    for (auto& [_, traj] : candidatesWithDist) {
        if (seenIds.insert(traj.getId()).second) {
            uniqueTrajs.push_back(traj);
            if (maxCount > 0 && uniqueTrajs.size() >= maxCount) break;
        }
    }

    outUnique = uniqueTrajs.size();
    return uniqueTrajs;
}

// ===== Range Query =====
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

// ===== Similarity Query =====
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

// ===== KNN Query =====
QueryStats Evaluation::runKNNQuery(const std::string& trajId, size_t k, int queryIndex) {
    QueryStats qs;
    qs.type = "kNearestNeighbors";
    qs.trajId = trajId;
    qs.k = k;

    const Trajectory* target = findTrajectoryById(trajId);
    if (!target) return qs;

    float timeScale = 1e-5f;

    // --- RTree KNN ---
    auto start = std::chrono::high_resolution_clock::now();
    auto rtreeResults = rtree.kNearestNeighbors(*target, k, timeScale);
    auto end = std::chrono::high_resolution_clock::now();
    qs.rtreeTime = std::chrono::duration<double>(end - start).count();

    rtreeResults = filterUniqueTrajectories(rtreeResults, target, k);
    qs.rtreeCount = rtreeResults.size();
    qs.rtreeUniqueVehicles = rtreeResults.size();

    // --- Linear scan KNN using unified linearScan ---
    start = std::chrono::high_resolution_clock::now();
    std::vector<Trajectory> linearResults = linearScan(
        [&](const Trajectory& t){ return t.getId() != target->getId(); }, // predicate
        target, qs.linearCount, qs.linearUniqueVehicles, k,
        [&](const Trajectory& t){ return target->spatioTemporalDistanceTo(t, timeScale); } // distance function
    );
    end = std::chrono::high_resolution_clock::now();
    qs.linearTime = std::chrono::duration<double>(end - start).count();

    saveQueryResults(queryIndex, qs.type, rtreeResults, linearResults);
    return qs;
}*/