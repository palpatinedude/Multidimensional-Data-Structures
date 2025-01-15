#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "interval_tree.h"

using namespace std;


//insert interval based on low value
IntervalTree::Node* IntervalTree::insert(Node* root,Interval i) {

    //if the root node is empty then make current node root
    if(root == nullptr)
    return newNode(i);

    //get root's low value
    long rlow = root->i->low;

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


//func to check if 2 given intervals overlap
bool IntervalTree::Overlap(Interval i1, Interval i2) {
    if(i1.low <= i2.high && i2.low <= i1.high)
    return true;
    return false;
}

