#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include <algorithm>
#include <stdexcept>

#include "interval_tree.h"
#include "interval_tree.cpp"

using json  = nlohmann::json;

//func to make intervals from json file made from dataset 
void loadFromJson(IntervalTree& tree, const std::string& processed_trajectories) {
    std::ifstream file(processed_trajectories);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + processed_trajectories);
    }

    json data = json::parse(file);

    for (const auto& trip : data["trajectories"]) {
        const auto& trajectory = trip["trajectory"];
        long long start = trajectory.front()["Timestamp"];
        long long end = trajectory.back()["Timestamp"];
        std::cout << "Inserting interval: [" << start << ", " << end << "]\n";  // Debugging line
        tree.insert(start, end);
    }
}

/* int main() {
IntervalTree tree;

    try {
        loadFromJson(tree, "../data/processed_trajectories.json");

        long long query_point = 1517356600000;  // Example timestamp for testing
        auto results = tree.stabbingQuery(query_point);

        // Check if results are empty
        if (results.empty()) {
            std::cout << "No intervals contain the point " << query_point << ".\n";
        } else {
            // Print results of the stabbing query
            std::cout << "Intervals containing the point " << query_point << ":\n";
            for (const auto& interval : results) {
                std::cout << "[" << interval.low << ", " << interval.high << "]\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}*/


/*int main() {
    IntervalTree tree;

    // Insert intervals
    tree.insert(1517356588000, 1517358617000);
    tree.insert(1517356590000, 1517356700000);

    // Display intervals before deletion
    std::cout << "Before deletion:\n";
    auto results = tree.stabbingQuery(1517356600000);
    for (const auto& interval : results) {
        std::cout << "[" << interval.low << ", " << interval.high << "]\n";
    }

    // Delete an interval using an Interval object
    IntervalTree::Interval toDelete = {1517356588000, 1517358617000};
    tree.deleteNode(toDelete);

    // Display intervals after deletion
    std::cout << "\nAfter deletion:\n";
    results = tree.stabbingQuery(1517356600000);
    for (const auto& interval : results) {
        std::cout << "[" << interval.low << ", " << interval.high << "]\n";
    }

    return 0;
}*/

/*int main() {
    IntervalTree tree;

    // Insert intervals
    std::cout << "Inserting intervals..." << std::endl;
    tree.insert(10, 20);
    tree.insert(5, 15);
    tree.insert(17, 25);
    tree.insert(3, 8);
    tree.insert(12, 18);

    // Print the tree
    std::cout << "\nInterval Tree after insertions:" << std::endl;
    tree.printIntervalTree();

    // Perform a stabbing query
    std::cout << "\nPerforming stabbing query for point 15:" << std::endl;
    std::vector<IntervalTree::Interval> result = tree.stabbingQuery(15);
    for (const auto& interval : result) {
        std::cout << "[" << interval.low << ", " << interval.high << "]" << std::endl;
    }

    // Perform a deletion
    std::cout << "\nDeleting interval [10, 20]..." << std::endl;
    tree.deleteNode({10, 20});

    // Print the tree again
    std::cout << "\nInterval Tree after deletion:" << std::endl;
    tree.printIntervalTree();

    // Perform another stabbing query
    std::cout << "\nPerforming stabbing query for point 17:" << std::endl;
    result = tree.stabbingQuery(17);
    for (const auto& interval : result) {
        std::cout << "[" << interval.low << ", " << interval.high << "]" << std::endl;
    }

    return 0;
}*/

int main() {
    IntervalTree tree;
    std::string filename = "C:\\project_multidimentional\\data\\processed_trajectories_final.json";

    try {
        // Load intervals from JSON
        loadFromJson(tree, filename);
        std::cout << "All intervals have been loaded into the Interval Tree.\n";

        // Interactive menu
        while (true) {
            std::cout << "\nOptions:\n"
                      << "1. Perform stabbing query (find intervals containing a point).\n"
                      << "2. Perform range query (find intervals overlapping a range).\n"
                      << "3. Print the Interval Tree.\n"
                      << "4. Exit.\n"
                      << "Enter your choice: ";
            int choice;
            std::cin >> choice;

            // Handle invalid input for choice
            if(std::cin.fail()) {
                std::cin.clear(); // clear error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard bad input
                std::cout << "Invalid choice. Please enter a valid number.\n";
                continue;
            }

            if (choice == 1) {
                long long point;
                std::cout << "Enter the point to query: ";
                std::cin >> point;

                if(std::cin.fail()) {
                    std::cin.clear(); 
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
                    std::cout << "Invalid input for point. Please enter a valid number.\n";
                    continue;
                }

                auto results = tree.stabbingQuery(point);
                if (results.empty()) {
                    std::cout << "No intervals contain the point " << point << ".\n";
                } else {
                    std::cout << "Intervals containing point " << point << ":\n";
                    for (const auto& interval : results) {
                        std::cout << "[" << interval.low << ", " << interval.high << "]\n";
                    }
                }
            } else if (choice == 2) {
                long long low, high;
                std::cout << "Enter the range to query (low high): ";
                std::cin >> low >> high;

                if(std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input for range. Please enter valid numbers.\n";
                    continue;
                }

                auto results = tree.rangeQuery(low, high);
                if (results.empty()) {
                    std::cout << "No intervals overlap with the range [" << low << ", " << high << "].\n";
                } else {
                    std::cout << "Intervals overlapping with the range [" << low << ", " << high << "]:\n";
                    for (const auto& interval : results) {
                        std::cout << "[" << interval.low << ", " << interval.high << "]\n";
                    }
                }
            } else if (choice == 3) {
                std::cout << "Printing the Interval Tree:\n";
                tree.printIntervalTree();
            } else if (choice == 4) {
                std::cout << "Exiting the program.\n";
                break;
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}