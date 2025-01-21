#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "interval_tree.h"


/*
// A utility function to create a new Interval Search Tree Node
IntervalTree::Node* IntervalTree::newNode(Interval i)
{
    Node* temp = new Node;
    temp->i = new Interval(i);
    temp->max = i.high;
    temp->left = temp->right = NULL;
    return temp;
};*/


//insert interval based on low value
IntervalTree::Node* IntervalTree::insert(Node* root,Interval i) {

    //if the root node is empty then make current node root
    if(root == nullptr)
    return new Node(i);

    //get root's low value
    long long rlow = root->i->low;

    //if current node's low < root's low go to left subtree
    if(i.low < rlow)
    root->left = insert(root->left, i);

    //else go to right subtree
    else 
    root->right = insert(root->right, i);

    //check max value and update if needed
    if(root->max < i.high)
    root-> max = i.high;

    return root;
}

void IntervalTree::insert(Interval i) {
    root = insert(root, i);
}


//second func for insert,overload insert
void IntervalTree::insert(long long low, long long high) {
    Interval i = {low, high};
    insert(i); 
}


//func to check if 2 given intervals overlap
bool IntervalTree::Overlap(Interval i1, Interval i2) const {
    if(i1.low <= i2.high && i2.low <= i1.high)
    return true;
    return false;
}

void IntervalTree::stabbingQuery(Node* root, long long point, std::vector<Interval>& result) const {
    if (!root)
        return;

    // Check if the point overlaps with the current node's interval using Overlap
    Interval query_interval = {point, point};  // Treat point as an interval [point, point]
    if (Overlap(*(root->i), query_interval)) {
        result.push_back(*(root->i));
    }

    // Go to left subtree if the point is within its range
    if (root->left && point <= root->left->max) {
        stabbingQuery(root->left, point, result);
    }

    // Else go to the right subtree
    stabbingQuery(root->right, point, result);
    }

    std::vector<IntervalTree::Interval> IntervalTree::stabbingQuery(long long point) {
    std::vector<Interval> result;
    stabbingQuery(root, point, result);
    return result;
}

//Function to delete all nodes in the tree
void IntervalTree::freeTree(Node* root) {
    if (!root)
        return;

    freeTree(root->left);  // Free the left subtree
    freeTree(root->right); // Free the right subtree

    delete root->i;  // Delete the interval
    delete root;     // Delete the node
}

IntervalTree::~IntervalTree() {
    freeTree(root);
}
