/*
main.cpp
--------
Purpose:
- Runs multiple convex hull algorithms in 2D and 3D.
- Uses the Evaluation class to measure time and memory usage.
- Saves points, hulls, and CSV results for later visualization.
- Supports multiple runs to compute averages and standard deviations.
*/

#include "evaluation/evaluation.h"  // Evaluation class for running experiments
#include "allAlgorithms.h"          // Implementations of convex hull algorithms
#include <iostream>
#include <vector>

int main() {
    // -----------------------------
    // Number of repetitions per input size
    int numRuns = 5;

    // Create evaluation object
    Evaluation evaluator(numRuns);

    // -----------------------------
    // Input sizes for experiments
    std::vector<int> sizes = {100, 500, 1000, 20000, 100000};

    // -----------------------------
    // Run 2D Divide & Conquer
    std::cout << "Running 2D Divide & Conquer..." << std::endl;
    evaluator.evaluate2D(divideAndConquer2D, sizes, "divideConquer2D");

    // Run 3D Chan's 
  //  std::cout << "Running 3D Chan's..." << std::endl;
    //evaluator.evaluate3D(chan3DConvexHull, sizes, "chan3D");

    // Run 2D Monotone Chain
    std::cout << "Running 2D Monotone Chain..." << std::endl;
    evaluator.evaluate2D(monotoneChainHull, sizes, "monotoneChain2D");

    // Run 2D QuickHull
    std::cout << "Running 2D QuickHull..." << std::endl;
    evaluator.evaluate2D(quickHull2D, sizes, "quickHull2D");

    // Run 2D Graham Scan
    std::cout << "Running 2D Graham Scan..." << std::endl;
    evaluator.evaluate2D(grahamScan2D, sizes, "grahamScan2D");

    // Run 3D QuickHull
    std::cout << "Running 3D QuickHull..." << std::endl;
    evaluator.evaluate3D(quickHull3D, sizes, "quickHull3D");

    return 0;
}

