#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class IntervalTree {

public:

//interval stucture
struct Interval {
    long long low;
    long long high;
};

private:

//node struct 
struct Node {
    Interval* i;
    long long max;
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
bool Overlap(Interval i1, Interval i2)const;
void stabbingQuery(Node* root, long long point, std::vector<Interval>& result) const;
void freeTree(Node* root);
Node* deleteNode(Node* root, Interval i); 
int getHeight(Node* root) const;


public:
    // Constructor and destructor
    IntervalTree() : root(nullptr) {}
    ~IntervalTree();

    // Public methods
    void insert(Interval i);
    void insert(long long low, long long high);  
    std::vector<Interval> stabbingQuery(long long point);
    void deleteNode(Interval i);
    void printIntervalTree() const;
};


#endif