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
Node(long long low, long long high, int driverCount = 0)
: interval{low, high},driverCount(0) , left(nullptr), right(nullptr) {}

//Destructor
~Node() {
delete left;
delete right;
}

//public functions
void updateDriverCount(const std::vector<std::tuple<long, long long, long long>>& driverTrips);

};


class SegmentTree {


private:
Node* root;

//private functions
    Node* buildTree(const std::vector<long long>& timestamps, 
                    const std::vector<std::tuple<long, long long, long long>>& driverTrips, 
                    int start, int end);

public:
 //Default Constructor
SegmentTree() : root(nullptr) {}

// Constructor to build the tree
SegmentTree(const std::vector<long long>& timestamps, 
    const std::vector<std::tuple<long, long long, long long>>& driverTrips);
    // Destructor
~SegmentTree() {
    delete root;
}

//public functions
int query(long long queryLow, long long queryHigh);

};

#endif