#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include <algorithm>
#include <stdexcept>

#include "interval_tree.h"
#include "interval_tree.cpp"

using json  = nlohmann::json;

//func to make intervals from json file made from dataset 
void loadFromJson(IntervalTree& tree, const std::string& processed_trajectories) {
    std::ifstream file(processed_trajectories);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + processed_trajectories);
    }

    json data = json::parse(file);

    for (const auto& trip : data["trajectories"]) {
        const auto& trajectory = trip["trajectory"];
        long long start = trajectory.front()["Timestamp"];
        long long end = trajectory.back()["Timestamp"];
        std::cout << "Inserting interval: [" << start << ", " << end << "]\n";  // Debugging line
        tree.insert(start, end);
    }
}

int main() {
    IntervalTree tree;

    try {
        loadFromJson(tree, "../data/processed_trajectories.json");

        long long query_point = 1230654222000;  // Example timestamp for testing
        auto results = tree.stabbingQuery(query_point);

        // Check if results are empty
        if (results.empty()) {
            std::cout << "No intervals contain the point " << query_point << ".\n";
        } else {
            // Print results of the stabbing query
            std::cout << "Intervals containing the point " << query_point << ":\n";
            for (const auto& interval : results) {
                std::cout << "[" << interval.low << ", " << interval.high << "]\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}

