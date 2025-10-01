#include <iostream>
#include "api/include/RTree.h"
#include "evaluation/evaluation.h"
#include <filesystem>
namespace fs = std::filesystem;

int main() {
    RTree rtree(8);

    // -----------------------------
    // Step 1: Load trajectories from Parquet
    // -----------------------------
    std::vector<Trajectory> trajectories;
    std::string parquetDir = "../preprocessing/trajectories_grouped.parquet";

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& entry : fs::directory_iterator(parquetDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".parquet") {
            auto partial = rtree.loadFromParquet(entry.path().string());
            trajectories.insert(trajectories.end(), partial.begin(), partial.end());
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> loadTime = end - start;
    std::cout << "Total trajectories loaded from Parquet: " << trajectories.size() << "\n";
    std::cout << "Loading completed in " << loadTime.count() << " seconds.\n";


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
    auto buildStart = std::chrono::high_resolution_clock::now();
    rtree.bulkLoad(trajectories);   // consumes trajectories
    auto buildEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> buildTime = buildEnd - buildStart;
    std::cout << "Bulk-load completed in " << buildTime.count() << " seconds.\n";

    // -----------------------------
    // Step 5: Initialize Evaluation AFTER bulkLoad
    // -----------------------------
    Evaluation eval(rtree, trajectoriesCopy, trajectoriesCopy, "results");

    // Export RTree to JSON (optional)
    rtree.exportToJSON("results/bulkloaded_tree.json");

    // Print statistics
    rtree.printStatistics();

    // -----------------------------
    // Step 6: Query loop
    // -----------------------------
    int numQueries;
    std::cout << "How many queries to run? ";
    std::cin >> numQueries;
    std::cin.ignore(); // flush newline

    std::vector<QueryStats> statsList;

    for (int i = 0; i < numQueries; ++i) {
        std::string queryType;
        std::cout << "Query type (rangeQuery, kNearestNeighbors, findSimilar): ";
        std::getline(std::cin, queryType);

        if (queryType == "rangeQuery") {
            std::string city, startTime, endTime;
            std::cout << "City (Philadelphia, Atlanta, Memphis): "; std::getline(std::cin, city);
            std::cout << "Start time (YYYY-MM-DDTHH:MM:SS): "; std::getline(std::cin, startTime);
            std::cout << "End time (YYYY-MM-DDTHH:MM:SS): "; std::getline(std::cin, endTime);

            statsList.push_back(eval.runRangeQuery(city, startTime, endTime, i + 1));

        } else if (queryType == "kNearestNeighbors") {
            std::string trajId;
            size_t k;
            std::cout << "Trajectory ID: ";
            std::getline(std::cin, trajId);
            std::cout << "k (number of neighbors): ";
            std::cin >> k;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush newline
            statsList.push_back(eval.runKNNQuery(trajId, k, i + 1));

        } else if (queryType == "findSimilar") {
            std::string trajId;
            float threshold;
            std::cout << "Trajectory ID: ";
            std::getline(std::cin, trajId);
            std::cout << "Similarity threshold: ";
            std::cin >> threshold;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush newline
            statsList.push_back(eval.runSimilarityQuery(trajId, threshold, i + 1));
        }
    }

    // -----------------------------
    // Step 6: Save summary
    // -----------------------------
    eval.saveSummary(statsList);

    return 0;
}
