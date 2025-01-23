#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include <queue>
#include <cmath>
#include <iomanip>

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



int IntervalTree::getHeight(Node* node) const {
    if (node == nullptr)
        return 0; //height of a null node is 0
    return node->height;
}


int IntervalTree::getBalanceFactor(IntervalTree::Node* node) {
    if (node == nullptr)
        return 0; // Base case: balance factor of a null node is 0
    return getHeight(node->left) - getHeight(node->right);
}


IntervalTree::Node* IntervalTree::rotateRight(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;

    // Update max values
    y->max = std::max(y->i->high,
                      std::max(y->left ? y->left->max : LLONG_MIN,
                               y->right ? y->right->max : LLONG_MIN));
    x->max = std::max(x->i->high,
                      std::max(x->left ? x->left->max : LLONG_MIN,
                               x->right ? x->right->max : LLONG_MIN));

    // Return new root
    return x;
}

IntervalTree::Node* IntervalTree::rotateLeft(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;

    // Update max values
    x->max = std::max(x->i->high,
                      std::max(x->left ? x->left->max : LLONG_MIN,
                               x->right ? x->right->max : LLONG_MIN));
    y->max = std::max(y->i->high,
                      std::max(y->left ? y->left->max : LLONG_MIN,
                               y->right ? y->right->max : LLONG_MIN));

    // Return new root
    return y;
}

IntervalTree::Node* IntervalTree::leftRightRotate(Node* node) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
}

IntervalTree::Node* IntervalTree::rightLeftRotate(Node* node) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
}


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

    //update height of the current node
    root->height = 1 + std::max(getHeight(root->left), getHeight(root->right));

    //check max value and update if needed
    if(root->max < i.high)
    root-> max = i.high;

    // Get the balance factor to check if this node is unbalanced
    int balance = getBalanceFactor(root);

    // Balance the tree based on the balance factor

    // Left Left Case
    if (balance > 1 && i.low < root->left->i->low)
        return rotateRight(root);

    // Right Right Case
    if (balance < -1 && i.low > root->right->i->low)
        return rotateLeft(root);

    // Left Right Case
    if (balance > 1 && i.low > root->left->i->low)
        return leftRightRotate(root);

    // Right Left Case
    if (balance < -1 && i.low < root->right->i->low)
        return rightLeftRotate(root);


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


IntervalTree::Node* IntervalTree::deleteNode(Node* root, Interval i) {
    if (!root) return nullptr;

    // Use Overlap to find the node to delete
    if (Overlap(*(root->i), i)) {
        // Node found; delete it

        // Case 1: Node with only one child or no child
        if (!root->left) {
            Node* temp = root->right;
            delete root->i;
            delete root;
            return temp;
        } else if (!root->right) {
            Node* temp = root->left;
            delete root->i;
            delete root;
            return temp;
        }

        // Case 2: Node with two children
        // Find the inorder successor (smallest in the right subtree)
        Node* temp = root->right;
        while (temp->left) temp = temp->left;

        // Copy the inorder successor's interval to this node
        *(root->i) = *(temp->i);

        // Delete the inorder successor
        root->right = deleteNode(root->right, *(temp->i));
    } else if (i.low < root->i->low) {
        // Recur into the left subtree
        root->left = deleteNode(root->left, i);
    } else {
        // Recur into the right subtree
        root->right = deleteNode(root->right, i);
    }

    //update height of the current node
    root->height = 1 + std::max(getHeight(root->left), getHeight(root->right));

    // Update the `max` value of this node
    root->max = std::max(root->i->high,
                         std::max(root->left ? root->left->max : LLONG_MIN,
                                  root->right ? root->right->max : LLONG_MIN));


    // Get the balance factor to check if this node is unbalanced
    int balance = getBalanceFactor(root);

    // Balance the tree based on the balance factor

    // Left Left Case
    if (balance > 1 && getBalanceFactor(root->left) >= 0)
        return rotateRight(root);

    // Left Right Case
    if (balance > 1 && getBalanceFactor(root->left) < 0)
        return leftRightRotate(root);

    // Right Right Case
    if (balance < -1 && getBalanceFactor(root->right) <= 0)
        return rotateLeft(root);

    // Right Left Case
    if (balance < -1 && getBalanceFactor(root->right) > 0)
        return rightLeftRotate(root);                              

    return root;
}

void IntervalTree::deleteNode(Interval i) {
    root = deleteNode(root, i);
}

void IntervalTree::printIntervalTree() const {
    if (!root) {
        std::cout << "Tree is empty." << std::endl;
        return;
    }

    // Helper function for printing the tree with root, left, and right subtree information
    auto printTree = [](Node* node, std::string prefix, bool isLeft, bool isRoot, auto& printTreeRef) -> void {
        if (!node) return;

        // Print the current node with special handling for root
        std::cout << prefix;

        if (isRoot) {
            std::cout << "**Root: ";
        } else if (isLeft) {
            std::cout << "|-- Left: ";
        } else {
            std::cout << "`-- Right: ";
        }

        // Print the interval and max value
        std::cout << "[" << node->i->low << ", " << node->i->high << "]"
                  << " (" << node->max << ")" << std::endl;

        // Prepare for the child nodes' prefixes
        std::string childPrefix = prefix + (isLeft ? "|   " : "    ");

        // Recursively print left and right subtrees with proper indication
        printTreeRef(node->left, childPrefix, true, false, printTreeRef);
        printTreeRef(node->right, childPrefix, false, false, printTreeRef);
    };

    // Start the recursive printing from root, which is marked as the root node
    printTree(root, "", false, true, printTree);
}
