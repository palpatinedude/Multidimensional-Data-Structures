#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include <queue>
#include <cmath>
#include <iomanip>
#include <tuple>

#include "segment_tree.h"

using namespace std;


// Function to count unique drivers in a node
void Node::updateDriverCount(const std::vector<std::tuple<long, long long, long long>>& driverTrips) {
    driverIds.clear(); // Clear existing driver IDs

    for (const auto& trip : driverTrips) {
        long driverId = std::get<0>(trip);
        long long tripStart = std::get<1>(trip);
        long long tripEnd = std::get<2>(trip);
        // Check if the driver's trip overlaps with the node's interval
        if (tripStart <= interval.high && tripEnd >= interval.low) {
            driverIds.insert(driverId);
        }
    }

    driverCount = driverIds.size(); // Update the driver count
}

//constructor for segment Tree
SegmentTree::SegmentTree(const std::vector<long long>& timestamps,
                        const std::vector<std::tuple<long, long long, long long>>& driverTrips) {
    if (timestamps.empty()) {
        throw std::invalid_argument("Timestamps vector cannot be empty.");
    }
    root = buildTree(timestamps, driverTrips, 0, timestamps.size() - 1);
}

//tree construction (bottom-up from timestamps)
Node* SegmentTree::buildTree(const std::vector<long long>& timestamps, 
                             const std::vector<std::tuple<long, long long, long long>>& driverTrips, 
                             int start, int end) {
    if (start > end) return nullptr;

    // Leaf node: Single timestamp
    if (start == end) {
        Node* leaf = new Node(timestamps[start], timestamps[start]);

        // Populate driver IDs for the leaf
        leaf->updateDriverCount(driverTrips);

        return leaf;
    }

    // Internal node
    int mid = start + (end - start) / 2;

    Node* leftChild = buildTree(timestamps, driverTrips, start, mid);
    Node* rightChild = buildTree(timestamps, driverTrips, mid + 1, end);

    Node* parent = new Node(leftChild->interval.low, rightChild->interval.high);
    parent->left = leftChild;
    parent->right = rightChild;

    // Populate driver IDs for the parent
    parent->updateDriverCount(driverTrips);

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