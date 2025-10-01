#include "evaluation.h"
#include <numeric>
#include <cmath>

// ----------------------
// numRuns: number of repetitions per input size
Evaluation::Evaluation(int numRuns) : numRuns(numRuns) {}

// ----------------------
// Compute mean 
double Evaluation::computeMean(const std::vector<double>& data) {
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

// ----------------------
// Compute standard deviation , given its mean
double Evaluation::computeStd(const std::vector<double>& data, double mean) {
    double sumSq = std::accumulate(data.begin(), data.end(), 0.0,
        [mean](double acc, double val) { return acc + (val - mean) * (val - mean); });
    return std::sqrt(sumSq / data.size());
}
