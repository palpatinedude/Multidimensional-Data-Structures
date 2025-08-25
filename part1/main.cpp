#include <iostream>
#include "api/include/RTree.h"
#include "evaluation/evaluation.h"

int main() {
    // -----------------------------
    // Step 1: Create RTree
    // -----------------------------
    RTree rtree(8);

    // -----------------------------
    // Step 2: Load trajectories from JSON
    // -----------------------------
    std::vector<Trajectory> trajectories = rtree.loadFromJSON(
        "preprocessing/trajectories_grouped.json"
    );

    // Copy for linear scan comparison
    std::vector<Trajectory> trajectoriesCopy = trajectories;

    // -----------------------------
    // Step 3: Bulk load into RTree
    // -----------------------------
    auto buildStart = std::chrono::high_resolution_clock::now();
    rtree.bulkLoad(trajectories);
    auto buildEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> buildTime = buildEnd - buildStart;
    std::cout << "Bulk-load completed in " << buildTime.count() << " seconds.\n";

    // Export RTree to JSON (optional)
    rtree.exportToJSON("results/bulkloaded_tree.json");

    // Print statistics
    rtree.printStatistics();

    // -----------------------------
    // Step 4: Initialize Evaluation
    // -----------------------------
    Evaluation eval(rtree, trajectories, trajectoriesCopy);
    float timeScale = 1e-5f; // consistent with Python version

    // -----------------------------
    // Step 5: Query loop
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

        auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\n\r"));
        s.erase(s.find_last_not_of(" \t\n\r") + 1);
        };

        std::cout << "Trajectory ID: "; 
        std::getline(std::cin, trajId);
        trim(trajId);  // remove leading/trailing whitespace/newlines
        std::cout << "[Debug] Searching for trajectory: '" << trajId << "'" << std::endl;

        std::cout << "k: "; 
        std::cin >> k; 
        std::cin.ignore();  // flush newline after reading k

        // --- New KNN call using spatio-temporal distance & unique trajectory IDs ---
        statsList.push_back(eval.runKNNQuery(trajId, k, i + 1));
        }else if (queryType == "findSimilar") {
            std::string trajId;
            float threshold;
            std::cout << "Trajectory ID: "; std::getline(std::cin, trajId);
            std::cout << "Threshold: "; std::cin >> threshold; std::cin.ignore();

            statsList.push_back(eval.runSimilarityQuery(trajId, threshold, i + 1));
        }
    }

    // -----------------------------
    // Step 6: Save summary
    // -----------------------------
    eval.saveSummary(statsList);

    return 0;
}
