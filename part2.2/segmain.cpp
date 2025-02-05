#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include <algorithm>
#include <stdexcept>

#include "segment_tree.h"
/*
using json  = nlohmann::json;

//func to make intervals from json file made from dataset 
void loadFromJson(SegmentTree& tree, const std::string& processed_trajectories) {
    std::ifstream file(processed_trajectories);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + processed_trajectories);
    }

    json data = json::parse(file);

    for (const auto& trip : data["trajectories"]) {
        const auto& trajectory = trip["trajectory"];
        int driverId = trip["driver_id"];

        if (trajectory.empty()) continue;  // Skip if trajectory is empty

        long long start = trajectory.front()["Timestamp"];
        long long end = trajectory.back()["Timestamp"];
    }
} */