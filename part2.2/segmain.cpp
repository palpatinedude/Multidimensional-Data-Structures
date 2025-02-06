#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include <algorithm>
#include <stdexcept>
#include <tuple>

#include "segment_tree.h"
#include "segment_tree.cpp"

using json  = nlohmann::json;

//The array in which the unique timestamps are going to be stored
    std::set<long long> uniqueTimestamps; 
//The vector in which the driver trips are going to be stored
std::vector<std::tuple<long, long long, long long>> driverTrips;


//func to get the needed data from the json file 
void loadFromJson(SegmentTree& tree, const std::string& processed_trajectories) {
    std::ifstream file(processed_trajectories);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + processed_trajectories);
    }

    json data = json::parse(file);

    /*
    //The array in which the unique timestamps are going to be stored
    std::set<long long> uniqueTimestamps; */

//Get the needed data from the json file
    for (const auto& trip : data["trajectories"]) {
        const auto& trajectory = trip["trajectory"];
        int driverId = trip["driver_id"];

        if (trajectory.empty()) continue;  // Skip if trajectory is empty

        long long start = trajectory.front()["Timestamp"]; //first timestamp
        long long end = trajectory.back()["Timestamp"]; //last timestamp
    

        uniqueTimestamps.insert(start);
        uniqueTimestamps.insert(end);
        driverTrips.emplace_back(driverId, start, end);


    }

   /* //unique timestamps are converted to a sorted vector
   std::vector<long long> sortedTimestamps(uniqueTimestamps.begin(), uniqueTimestamps.end());
  */
}

int main(){

    SegmentTree tree;
    std::string filename = "C:\\project_multidimentional\\data\\processed_trajectories4.json";

    try {
        // Load from JSON
        loadFromJson(tree, filename);

        //unique timestamps are converted to a sorted vector
        std::vector<long long> sortedTimestamps(uniqueTimestamps.begin(), uniqueTimestamps.end());

        std::cout << "All needed info have been loaded into the SegmentTree.\n";

        // Build the segment tree with the given data
        SegmentTree tree(sortedTimestamps, driverTrips);

}catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
        }

return EXIT_SUCCESS;
}