#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <memory>
#include <set>

class Node
{

public:
//interval stucture
struct Interval {
    long long low;
    long long high;
};

Interval interval;
std::set<long> driverIds;  // Stores unique driver IDs in this interval
int driverCount;  // Stores the count of unique driver IDs in this interval
Node* left;
Node* right;

//Constructor
Node(long long low, long long high, long long val = 0)
: interval{low, high},driverCount(0) , left(nullptr), right(nullptr) {}

//Destructor
~Node() {
delete left;
delete right;
}

//public functions
int countDrivers();

};


class SegmentTree {


private:
Node* root;

//private functions
Node* buildTree(const std::vector<long long>& timestamps, int start, int end);

public:

  // Constructor 
SegmentTree(const std::vector<long long>& timestamps) {
    std::vector<long long> sortedTimestamps = timestamps;

    // Sort timestamps to maintain correct order
    std::sort(sortedTimestamps.begin(), sortedTimestamps.end());

    // Build the tree bottom-up
    root = buildTree(sortedTimestamps, 0, sortedTimestamps.size() - 1);
}
    // Destructor
~SegmentTree() {
    delete root;
}

//public functions
int query(long long queryLow, long long queryHigh);

};

#endif