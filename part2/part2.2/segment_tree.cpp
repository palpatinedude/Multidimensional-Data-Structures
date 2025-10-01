// segment_tree.cpp - Implementation of Trip Counting Segment Tree
#include "segment_tree.h"
#include <algorithm>
#include <iostream>

/**
 * Constructor: Builds the segment tree from timestamps and trip data
 * Time Complexity: O(n log n) where n = number of timestamps
 */
SegmentTree::SegmentTree(const std::vector<long long>& timestamps, 
                        const std::vector<std::tuple<long, long long, long long>>& tripData) 
    : trips(tripData) {
    
    if (timestamps.empty()) {
        throw std::invalid_argument("Timestamps cannot be empty");
    }
    
    // Sort timestamps and remove duplicates to create discrete time points
    // This ensures our segment tree covers only relevant time points
    std::vector<long long> sortedTimestamps = timestamps;
    std::sort(sortedTimestamps.begin(), sortedTimestamps.end());
    sortedTimestamps.erase(std::unique(sortedTimestamps.begin(), sortedTimestamps.end()), 
                          sortedTimestamps.end());
    
    // Build tree using indices into the sorted timestamp array
    // Tree will have sortedTimestamps.size() leaf nodes
    root = buildTree(sortedTimestamps, 0, sortedTimestamps.size() - 1);
}

/**
 * Recursively builds segment tree with proper leaf/internal node structure
 * Each leaf represents a single timestamp, internal nodes represent intervals
 */
Node* SegmentTree::buildTree(const std::vector<long long>& timestamps, int start, int end) {
    // Base case: Create leaf node for single timestamp
    // Leaf nodes represent the smallest discrete time units in our tree
    if (start == end) {
        Node* leaf = new Node(timestamps[start], timestamps[start]);
        // Count trips that include this specific timestamp
        leaf->tripCount = countOverlappingTrips(timestamps[start], timestamps[start]);
        return leaf;
    }
    
    // Recursive case: Create internal node covering range [start, end]
    int mid = start + (end - start) / 2;  // Avoid integer overflow
    Node* node = new Node(timestamps[start], timestamps[end]);
    
    // Recursively build left and right subtrees
    node->left = buildTree(timestamps, start, mid);          // Left half: [start, mid]
    node->right = buildTree(timestamps, mid + 1, end);       // Right half: [mid+1, end]
    
    // Count trips overlapping the entire interval represented by this internal node
    // This covers the time span from timestamps[start] to timestamps[end]
    node->tripCount = countOverlappingTrips(timestamps[start], timestamps[end]);
    
    return node;
}

/**
 * Counts trips that overlap with a given time interval
 * A trip overlaps an interval if: tripStart <= intervalEnd AND tripEnd >= intervalStart
 */
int SegmentTree::countOverlappingTrips(long long intervalStart, long long intervalEnd) {
    int count = 0;
    
    // Check each trip for overlap with the given interval
    for (const auto& trip : trips) {
        long long tripStart = std::get<1>(trip);  // Extract start time from tuple
        long long tripEnd = std::get<2>(trip);    // Extract end time from tuple
        
        // Standard interval overlap check:
        // Two intervals [a,b] and [c,d] overlap if: a <= d AND b >= c
        if (tripStart <= intervalEnd && tripEnd >= intervalStart) {
            count++;
        }
    }
    return count;
}

/**
 * Recursive helper function for range queries
 * Traverses tree to find nodes that overlap with query range
 */
int SegmentTree::queryHelper(Node* node, long long queryStart, long long queryEnd) {
    // Base case: null node
    if (!node) return 0;
    
    // Case 1: No overlap between node's interval and query range
    if (node->end < queryStart || node->start > queryEnd) {
        return 0;
    }
    
    // Case 2: Any overlap - count trips that overlap with the query range
    // This handles both complete and partial overlap cases correctly
    // Instead of using precomputed tripCount, we count overlaps with actual query range
    
    if (node->left == nullptr && node->right == nullptr) {
        // Leaf node: count trips that overlap with query range
        int count = 0;
        for (const auto& trip : trips) {
            long long tripStart = std::get<1>(trip);
            long long tripEnd = std::get<2>(trip);
            
            // Check if trip overlaps with query range [queryStart, queryEnd]
            if (tripStart <= queryEnd && tripEnd >= queryStart) {
                count++;
            }
        }
        return count;
    } else {
        // Internal node: recurse to children and take maximum
        int leftResult = node->left ? queryHelper(node->left, queryStart, queryEnd) : 0;
        int rightResult = node->right ? queryHelper(node->right, queryStart, queryEnd) : 0;
        
        // Return maximum because we want the count of trips active during the query period
        return std::max(leftResult, rightResult);
    }
}


/**
 * Public query interface
 * Returns count of trips active during the specified time range
 */
int SegmentTree::query(long long queryStart, long long queryEnd) {
    // Validate input
    if (queryStart > queryEnd) return 0;
    
    // Start recursive query from root
    return queryHelper(root, queryStart, queryEnd);
}