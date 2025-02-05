#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include <queue>
#include <cmath>
#include <iomanip>
#include "C:\project_multidimentional\part2.1\json.hpp"

#include "segment_tree.h"

using namespace std;


using json  = nlohmann::json;

//func to make intervals from json file made from dataset 
void loadFromJson(SegmentTree& tree, const std::string& processed_trajectories) {
    std::ifstream file(processed_trajectories);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + processed_trajectories);
    }

    json data = json::parse(file);

    //The array in which the unique timestamps are stored
    std::set<long long> uniqueTimestamps; 

//Get the needed data from the json file
    for (const auto& trip : data["trajectories"]) {
        const auto& trajectory = trip["trajectory"];
        int driverId = trip["driver_id"];

        if (trajectory.empty()) continue;  // Skip if trajectory is empty

        long long start = trajectory.front()["Timestamp"]; //first timestamp
        long long end = trajectory.back()["Timestamp"]; //last timestamp
    

        uniqueTimestamps.insert(start);
        uniqueTimestamps.insert(end);

    }

    //unique timestamps are converted to a sorted vector
   std::vector<long long> sortedTimestamps(uniqueTimestamps.begin(), uniqueTimestamps.end());
  
}

// Function to count unique drivers in a node
void Node::updateDriverCount() {
    driverCount = driverIds.size();
}

//tree construction (bottom-up from timestamps)
    Node* buildTree(const std::vector<long long>& timestamps,
                    const std::vector<std::tuple<long, long long, long long>>& driverTrips,
                    int start, int end) {
        if (start > end) return nullptr;

        // Leaf node: Single timestamp
        if (start == end) {
            Node* leaf = new Node(timestamps[start], timestamps[start]);
            
            // Add drivers whose trips overlap with this timestamp
            for (const auto& [driverId, tripStart, tripEnd] : driverTrips) {
                if (tripStart <= leaf->interval.low && leaf->interval.high <= tripEnd) {
                    leaf->driverIds.insert(driverId);
                }
            }
            // Update driver count
            leaf->updateDriverCount();
            return leaf;
        }

        // Middle index
        int mid = start + (end - start) / 2;

        // Recursively build left and right subtrees
        Node* leftChild = buildTree(timestamps, driverTrips, start, mid);
        Node* rightChild = buildTree(timestamps, driverTrips, mid + 1, end);

        // Create parent node merging child intervals
        Node* parent = new Node(leftChild->interval.low, rightChild->interval.high);
        parent->left = leftChild;
        parent->right = rightChild;

        // Merge driver IDs from children
        parent->driverIds.insert(leftChild->driverIds.begin(), leftChild->driverIds.end());
        parent->driverIds.insert(rightChild->driverIds.begin(), rightChild->driverIds.end());

        // Update driver count
        parent->updateDriverCount();

        return parent;
    }
/*Node* SegmentTree::buildTree(const std::vector<long long>& timestamps, int start, int end) {
    if (start > end) return nullptr;

    // Leaf node: Single timestamp as interval [low = high]
    if (start == end) {
        return new Node(timestamps[start], timestamps[start]);
    }

    // Find the middle index
    int mid = start + (end - start) / 2;

    // Recursively build left and right subtrees
    Node* leftChild = buildTree(timestamps, start, mid);
    Node* rightChild = buildTree(timestamps, mid + 1, end);

    // Parent node merges child timestamps into an interval
    Node* parent = new Node(leftChild->interval.low, rightChild->interval.high);
    parent->left = leftChild;
    parent->right = rightChild;

    return parent;
}*/

int SegmentTree::query(long long queryLow, long long queryHigh) {
    std::set<long> uniqueDrivers; // Store unique driver IDs

    std::function<void(Node*)> searchTree = [&](Node* node) {
        if (!node) return;

        // If current interval is completely inside query range, add all drivers
        if (queryLow <= node->interval.low && node->interval.high <= queryHigh) {
            uniqueDrivers.insert(node->driverIds.begin(), node->driverIds.end());
            return;
        }

        // If query range overlaps, check children
        if (node->left && node->interval.low <= queryHigh) searchTree(node->left);
        if (node->right && node->interval.high >= queryLow) searchTree(node->right);
    };

    searchTree(root);

    return uniqueDrivers.size(); // Return count of unique drivers
}