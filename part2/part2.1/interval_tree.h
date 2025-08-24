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
    int height; // Height of the node optimal way

    // Constructor for Node
        Node(Interval interval)
            : i(new Interval(interval)), max(interval.high),height(1), left(nullptr), right(nullptr) {}
        ~Node() { delete i; }
};


Node* root;

//private methods
Node* insert(Node* root,Interval i);
bool Overlap(Interval i1, Interval i2)const;
void stabbingQuery(Node* root, long long point, std::vector<Interval>& result) const;
void freeTree(Node* root);
Node* deleteNode(Node* root, Interval i); 
void rangeQuery(Node* root, Interval query, std::vector<Interval>& result) const;

//To balance the tree
int getHeight(Node* node) const;
int getBalanceFactor(Node* node);
Node* rotateRight(Node* y);
Node* rotateLeft(Node* x);
Node* leftRightRotate(Node* node);
Node* rightLeftRotate(Node* node);


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
    std::vector<Interval> rangeQuery(long long low, long long high);
};


#endif