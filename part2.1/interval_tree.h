#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class IntervalTree {

private:    

//interval stucture
struct Interval {
    long low;
    long high;
};

//node struct 
struct Node {
    Interval* i;
    long max;
    Node* left;
    Node* right;

    // Constructor for Node
        Node(Interval interval)
            : i(new Interval(interval)), max(interval.high), left(nullptr), right(nullptr) {}
        ~Node() { delete i; }
};


Node* root;

//private methods
Node* insert(Node* root,Interval i);
bool Overlap(Interval i1, Interval i2);
void stabbingQuery(Node* root, long point, std::vector<Interval>& result) const;
void deletation(Node* root);


public:
    // Constructor and destructor
    IntervalTree() : root(nullptr) {}
    ~IntervalTree();

    // Public methods
    void insert(Interval i);
    void insert(long low, long high);  
    std::vector<Interval> stabbingQuery(long point);
};


#endif