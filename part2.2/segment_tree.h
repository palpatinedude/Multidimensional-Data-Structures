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

int countDrivers() {
    driverCount = driverIds.size(); //get the number of driver ids
    return driverCount;
}

};

class SegmentTree {


private:
Node* root;



};

#endif