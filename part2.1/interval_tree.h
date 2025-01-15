#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

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
};

// A utility function to create a new Interval Search Tree Node
Node* newNode(Interval i)
{
    Node* temp = new Node;
    temp->i = new Interval(i);
    temp->max = i.high;
    temp->left = temp->right = NULL;
    return temp;
};

Node* root;

//private methods
Node* insert(Node* root,Interval i);
bool Overlap(Interval i1, Interval i2);

public:
    // Constructor and destructor
    IntervalTree() : root(nullptr) {}
    ~IntervalTree();

    // Public methods
    void insert(Interval i);
    std::vector<Interval> stabbingQuery(long point);
};


#endif