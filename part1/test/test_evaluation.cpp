#include "../api/include/RTree.h"
#include "../api/include/trajectory.h"
#include "../api/include/bbox3D.h"
#include "../api/include/point3D.h"
#include "../api/include/RTreeNode.h"
#include "../evaluation/evaluation.h"

#include <iostream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// ---------------- Load Parquet trajectories ----------------
std::vector<Trajectory> loadTrajectoriesFromParquet(const std::string& parquetDir) {
    std::vector<Trajectory> trajectories;

    if (!fs::exists(parquetDir) || !fs::is_directory(parquetDir)) {
        std::cerr << "[Error] Parquet directory does not exist: " << parquetDir << "\n";
        return trajectories;
    }

    for (const auto& entry : fs::directory_iterator(parquetDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".parquet") {
            auto partial = RTree::loadFromParquet(entry.path().string());
            trajectories.insert(trajectories.end(), partial.begin(), partial.end());
        }
    }

    std::cout << "Loaded " << trajectories.size() << " trajectories from Parquet files\n";
    return trajectories;
}

// ---------------- Run Range Queries ----------------
void runRangeQueries(Evaluation& eval) {
    std::cout << "\n=== Range Query Tests ===\n";
    int queryIndex = 0;

    std::vector<std::pair<std::string,std::string>> timeRanges = {
        {"2017-01-01T00:00:00Z","2017-06-01T00:00:00Z"},
        {"2017-06-01T00:00:00Z","2018-01-01T00:00:00Z"}
    };
    std::vector<std::string> cities = {"Philadelphia","Atlanta","Memphis"};

    for (auto& city : cities) {
        for (auto& tr : timeRanges) {
            auto qs = eval.runRangeQuery(city, tr.first, tr.second, queryIndex++);
            std::cout << "RangeQuery " << city << " [" << tr.first << " - " << tr.second << "] returned "
                      << qs.rtreeCount << " RTree, " << qs.linearCount << " Linear trajectories\n";
        }
    }
}

// ---------------- Run kNN Queries ----------------
void runKNNQueries(Evaluation& eval, const std::vector<Trajectory>& trajectories) {
    std::cout << "\n=== kNN Query Tests ===\n";
    int queryIndex = 0;
    std::vector<size_t> ks = {5, 10, 50};
    std::string testID = trajectories[1].getId();
    for (size_t k : ks) {
        for (auto& t : trajectories) {
            if (t.getId() == testID ) { // limit to a few
                auto qs = eval.runKNNQuery(t.getId(), k, queryIndex++);
                std::cout << "kNN " << t.getId() << " k=" << k
                          << " RTree: " << qs.rtreeCount
                          << " Linear: " << qs.linearCount << "\n";
            }
        }
    }
}

// ---------------- Run Similarity Queries ----------------
void runSimilarityQueries(Evaluation& eval, const std::vector<Trajectory>& trajectories) {
    std::cout << "\n=== Similarity Query Tests ===\n";
    int queryIndex = 0;
    std::vector<float> thresholds = {0.001f, 0.01f, 0.05f};
    std::string testID = trajectories[1].getId();
    for (auto& t : trajectories) {
        if (t.getId() == testID) { // limit to a few
            for (float thr : thresholds) {
                auto qs = eval.runSimilarityQuery(t.getId(), thr, queryIndex++);
                std::cout << "Similarity " << t.getId() << " threshold=" << thr
                          << " RTree: " << qs.rtreeCount
                          << " Linear: " << qs.linearCount << "\n";
            }
        }
    }
}

// ---------------- Main ----------------
int main() {
    std::cout << "=== Evaluation Test on Real Parquet Data ===\n";
    RTree rtree(8);

    std::vector<Trajectory> trajectories;
    std::string parquetDir = "../../preprocessing/trajectories_grouped.parquet";


    for (const auto& entry : fs::directory_iterator(parquetDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".parquet") {
            auto partial = rtree.loadFromParquet(entry.path().string());
            trajectories.insert(trajectories.end(), partial.begin(), partial.end());
        }
    }

    std::cout << "Total trajectories loaded from Parquet: " << trajectories.size() << "\n";



    // -----------------------------
    // Step 2: Precompute centroids & bounding boxes
    // -----------------------------
    for (auto& traj : trajectories) {
    traj.precomputeCentroidAndBoundingBox();
    }

    // -----------------------------
    // Step 3: Keep a copy for linear scan
    // -----------------------------
    std::vector<Trajectory> trajectoriesCopy = trajectories;

    // -----------------------------
    // Step 4: Bulk-load into RTree
    // -----------------------------

    rtree.bulkLoad(trajectories);   // consumes trajectories
    std::cout << "Bulk-load completed.\n";

    // -----------------------------
    // Step 5: Initialize Evaluation AFTER bulkLoad
    // -----------------------------
    Evaluation eval(rtree, trajectoriesCopy, trajectoriesCopy, "results");
    // Run controlled tests
    runRangeQueries(eval);
    runKNNQueries(eval, trajectories);
    runSimilarityQueries(eval, trajectories);

    std::cout << "\n=== Evaluation on Parquet Data Completed ===\n";
    return 0;
}
