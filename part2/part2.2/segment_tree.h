// segment_tree.h - Header file for Trip Counting Segment Tree
#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

#include <vector>
#include <tuple>
#include <functional>

/**
 * Node class represents a single node in the segment tree
 * Each node covers a time interval and stores the count of trips overlapping that interval
 */
class Node {
public:
    long long start, end;  // Time interval this node represents [start, end]
    int tripCount;         // Number of trips overlapping this time interval
    Node* left;            // Pointer to left child (earlier time intervals)
    Node* right;           // Pointer to right child (later time intervals)

    // Constructor: Initialize node with time interval
    Node(long long s, long long e) : start(s), end(e), tripCount(0), left(nullptr), right(nullptr) {}
    
    // Destructor: Recursively delete all child nodes
    ~Node() {
        delete left;
        delete right;
    }
};

/**
 * SegmentTree class for efficiently counting trips in time intervals
 * Built on discrete timestamps, supports range queries for trip counts
 */
class SegmentTree {
private:
    Node* root;  // Root node of the segment tree
    std::vector<std::tuple<long, long long, long long>> trips;  // Store all trip data for counting
    
    // Private helper functions
    
    /**
     * Recursively builds the segment tree
     * @param timestamps: Sorted array of unique timestamps
     * @param start: Starting index in timestamps array
     * @param end: Ending index in timestamps array
     * @return: Pointer to root of subtree covering timestamps[start..end]
     */
    Node* buildTree(const std::vector<long long>& timestamps, int start, int end);
    
    /**
     * Counts how many trips overlap with given time interval
     * @param intervalStart: Start time of interval
     * @param intervalEnd: End time of interval  
     * @return: Number of trips that overlap with [intervalStart, intervalEnd]
     */
    int countOverlappingTrips(long long intervalStart, long long intervalEnd);
    
    /**
     * Recursive helper function for range queries
     * @param node: Current node being examined
     * @param queryStart: Start of query time range
     * @param queryEnd: End of query time range
     * @return: Number of trips active during query range
     */
    int queryHelper(Node* node, long long queryStart, long long queryEnd);

public:
    /**
     * Constructor: Builds segment tree from timestamps and trip data
     * @param timestamps: Vector of discrete timestamp points
     * @param tripData: Vector of tuples (tripId, startTime, endTime)
     */
    SegmentTree(const std::vector<long long>& timestamps, 
                const std::vector<std::tuple<long, long long, long long>>& tripData);
    
    // Destructor: Clean up all allocated memory
    ~SegmentTree() { delete root; }
    
    /**
     * Query how many trips were active during given time range
     * @param queryStart: Start time of query range
     * @param queryEnd: End time of query range
     * @return: Number of trips overlapping with [queryStart, queryEnd]
     */
    int query(long long queryStart, long long queryEnd);
};

#endif