#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include <queue>
#include <cmath>
#include <iomanip>

#include "segment_tree.h"

using namespace std;


// Function to count unique drivers in a node
int Node::countDrivers() {
    driverCount = driverIds.size();
    return driverCount;
}

// Efficient tree construction (bottom-up from timestamps)
Node* SegmentTree::buildTree(const std::vector<long long>& timestamps, int start, int end) {
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
}

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