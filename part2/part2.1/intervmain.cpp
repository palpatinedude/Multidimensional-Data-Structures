#include <iostream>
#include "interval_tree.h"

int main() {
    IntervalTree tree;

    // Insert some intervals
    tree.insert(5, 20);
    tree.insert(10, 30);
    tree.insert(12, 15);
    tree.insert(17, 19);
    tree.insert(30, 40);

    std::cout << "Initial Interval Tree:\n";
    tree.printIntervalTree();
    std::cout << "------------------------\n";

    // Stabbing query: intervals containing a point
    long long point = 14;
    auto stabbingResult = tree.stabbingQuery(point);
    std::cout << "Intervals containing point " << point << ":\n";
    for (auto &i : stabbingResult) {
        std::cout << "[" << i.low << ", " << i.high << "]\n";
    }
    std::cout << "------------------------\n";

    // Range query: intervals overlapping with [14, 18]
    long long queryLow = 14, queryHigh = 18;
    auto rangeResult = tree.rangeQuery(queryLow, queryHigh);
    std::cout << "Intervals overlapping with [" << queryLow << ", " << queryHigh << "]:\n";
    for (auto &i : rangeResult) {
        std::cout << "[" << i.low << ", " << i.high << "]\n";
    }
    std::cout << "------------------------\n";

    // Delete an interval
    tree.deleteNode({10, 30});
    std::cout << "Interval Tree after deleting [10, 30]:\n";
    tree.printIntervalTree();
    std::cout << "------------------------\n";

    return 0;
}
