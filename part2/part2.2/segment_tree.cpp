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
#include <unordered_set>

#include "segment_tree.h"

using namespace std;



// Function to update driver count in each node
void Node::updateDriverCount(const std::vector<std::tuple<long, long long, long long>>& driverTrips) {
    driverIds.clear(); // Reset existing driver IDs

    for (const auto& trip : driverTrips) {
        long driverId = std::get<0>(trip);
        long long tripStart = std::get<1>(trip);
        long long tripEnd = std::get<2>(trip);

        // Check if this trip overlaps with the node's interval
        if (tripStart <= interval.high && tripEnd >= interval.low) {
            driverIds.insert(driverId);
        }
    }

    // Set the driver count
    driverCount = driverIds.size();

    // Aggregate counts from child nodes (for internal nodes)
    if (left) driverCount += left->driverCount;
    if (right) driverCount += right->driverCount;
}

//constructor for segment Tree
SegmentTree::SegmentTree(std::vector<long long>& timestamps,
    std::vector<std::tuple<long, long long, long long>>& driverTrips)  {
    if (timestamps.empty()) {
        throw std::invalid_argument("Timestamps vector cannot be empty.");
    }
    root = buildTree(timestamps, driverTrips, 0, timestamps.size() - 1);
}

//Build tree function (bottom-up from timestamps)
Node* SegmentTree::buildTree(const std::vector<long long>& timestamps, 
                             const std::vector<std::tuple<long, long long, long long>>& driverTrips, 
                             long long start, long long end) {
    if (start > end) return nullptr;

    // Leaf node: Single timestamp
    if (start == end) {
        Node* leaf = new Node(timestamps[start], timestamps[start]);
        leaf->updateDriverCount(driverTrips);
        return leaf;
    }

    // Internal node: Split range
    long long mid = start + (end - start) / 2;

    Node* leftChild = buildTree(timestamps, driverTrips, start, mid);
    Node* rightChild = buildTree(timestamps, driverTrips, mid + 1, end);

    Node* parent = new Node(leftChild->interval.low, rightChild->interval.high);
    parent->left = leftChild;
    parent->right = rightChild;

    // Update driver count by merging child nodes
    parent->updateDriverCount(driverTrips);

    return parent;
}

int SegmentTree::query(long long queryLow, long long queryHigh) {
    std::unordered_set<long> uniqueDrivers;

    std::function<void(Node*)> searchTree = [&](Node* node) {
        if (!node) return;

        // If query range completely contains this node's range
        if (queryLow <= node->interval.low && node->interval.high <= queryHigh) {
            uniqueDrivers.insert(node->driverIds.begin(), node->driverIds.end());
            return;
        }

        // If query range partially overlaps, recurse
        if (node->left && node->left->interval.high >= queryLow) searchTree(node->left);
        if (node->right && node->right->interval.low <= queryHigh) searchTree(node->right);
    };

    searchTree(root);
    return uniqueDrivers.size();
}