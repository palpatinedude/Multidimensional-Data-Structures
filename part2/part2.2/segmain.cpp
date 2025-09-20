#include <iostream>
#include <fstream>
#include <vector>
#include <set> // Include for std::set
#include <algorithm>
#include <stdexcept>
#include <tuple>

#include "C:\\project_multidimentional\\part2.2\\segment_tree.h"
#include "C:\\project_multidimentional\\part2.1\\json.hpp"

using json  = nlohmann::json;

using namespace std;

// Function to load data from JSON and store it in a SegmentTree
SegmentTree loadFromJson(const std::string& processed_trajectories, 
                         std::vector<long long>& sortedTimestamps, 
                         std::vector<std::tuple<long, long long, long long>>& driverTrips) {

    std::set<long long> uniqueTimestamps;  // Set to collect unique timestamps

    std::ifstream file(processed_trajectories);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + processed_trajectories);
    }

    json data = json::parse(file);

    // Extract data from JSON
    for (const auto& trip : data["trajectories"]) {
        const auto& trajectory = trip["trajectory"];
        int driverId = trip["driver_id"];

        if (trajectory.empty()) continue;  // Skip if trajectory is empty

        long long start = trajectory.front()["Timestamp"];  // first timestamp
        long long end = trajectory.back()["Timestamp"];    // last timestamp

        uniqueTimestamps.insert(start);
        uniqueTimestamps.insert(end);
        driverTrips.emplace_back(driverId, start, end);
    }

    // Convert the set of unique timestamps into a sorted vector
    sortedTimestamps.assign(uniqueTimestamps.begin(), uniqueTimestamps.end());

    return SegmentTree(sortedTimestamps, driverTrips);
}

int main() {
    std::string filename = "C:\\project_multidimentional\\data\\processed_trajectories4.json";
    std::vector<long long> sortedTimestamps;
    std::vector<std::tuple<long, long long, long long>> driverTrips;

    try {
        // Load timestamps and driver trips from the JSON file
        SegmentTree tree = loadFromJson(filename, sortedTimestamps, driverTrips);

        std::cout << "All needed info have been loaded into the SegmentTree.\n";

        // Example query
        long long queryLow = 1610000000;
        long long queryHigh = 1615000000;
        int driverCount = tree.query(queryLow, queryHigh);
        std::cout << "Drivers in interval [" << queryLow << ", " << queryHigh << "]: " << driverCount << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
