#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include <algorithm>
#include <stdexcept>

#include "interval_tree.h"
#include "data\processed_trajectories.json"

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
        long start = trajectory.front()["Timestamp"];
        long end = trajectory.back()["Timestamp"];
        tree.insert({start, end});
    }
}

int main() {
    IntervalTree tree;

    try {
        loadFromJson(tree, "processed_trajectories.json");
    }

    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}