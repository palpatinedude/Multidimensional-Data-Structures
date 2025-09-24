/*
evaluation.h
------------
Class for evaluating convex hull algorithms (2D & 3D).  

Purpose:
- Runs multiple experiments for a given algorithm and input sizes.
- Computes average runtime and memory usage with standard deviations.
- Provides a reusable interface for both 2D and 3D algorithms.
- Designed as a template class so any compatible convex hull function can be passed.

*/

#ifndef EVALUATION_H
#define EVALUATION_H

#include "../common.h"
#include "../allAlgorithms.h"
#include <vector>
#include <string>

class Evaluation {
public:
    // Constructor: set number of runs per input size (default = 5)
    Evaluation(int numRuns = 5);

    // ----------------------
    // Template: evaluate 2D convex hull algorithm
    // hullFunc: algorithm function (vector<Point> -> vector<Point>)
    // sizes: list of input sizes to test
    // algName: identifier/name for the algorithm (used in results)
    template<typename Func>
    void evaluate2D(Func hullFunc,
                    const std::vector<int>& sizes,
                    const std::string& algName);

    // ----------------------
    // Template: evaluate 3D convex hull algorithm
    // hullFunc: algorithm function (vector<Point3> -> vector<Point3> or Hull3D)
    // sizes: list of input sizes to test
    // algName: identifier/name for the algorithm (used in results)
    template<typename Func>
    void evaluate3D(Func hullFunc,
                    const std::vector<int>& sizes,
                    const std::string& algName);

private:
    int numRuns; // number of repetitions per input size

    // ----------------------
    // Helper: compute mean of a dataset
    double computeMean(const std::vector<double>& data);

    // Helper: compute standard deviation given mean
    double computeStd(const std::vector<double>& data, double mean);
};

// Include template implementations
#include "evaluation.tpp" 

#endif // EVALUATION_H
