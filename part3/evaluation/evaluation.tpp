/*
evaluation.tpp
---------------
Template implementations for the Evaluation class.

Purpose:
- Runs multiple experiments on 2D/3D convex hull algorithms.
- Measures time (in microseconds) and memory usage (KB) for varying input sizes.
- Memory for 2D : The total memory is the size of one 2D point multiplied by the sum of the number of points and the number of points in the hull.
- Memory for 3D : The total memory is the size of one 3D point multiplied by the sum of the number of points and the number of vertices in the hull.
- Supports multiple runs to compute average and standard deviation.
- Stores results in CSV and saves point/hull files for visualization.


- 2D experiments generate random 2D points.
- 3D experiments generate either random 3D points or points on a sphere (for QuickHull3D large n).
- Use `numRuns` to set repetitions for averaging.
- Single-run optimization is used for very large 3D QuickHull inputs.
*/

#ifndef EVALUATION_TPP
#define EVALUATION_TPP

#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;
using namespace chrono;

// ------------------ 2D Experiment ------------------
template<typename Func>
void Evaluation::evaluate2D(Func hullFunc,
                            const vector<int>& sizes,
                            const string& algName) {
    // Ensure result directory exists
    fs::create_directories("results/" + algName);

    // Open CSV to log results
    ofstream out("results/" + algName + "/" + algName + "_results.csv");
    out << "n,avg_time_us,std_time_us,avg_mem_KB,std_mem_KB\n";

    for (int n : sizes) {
        vector<double> times, mems;

        // Repeat experiment multiple times for statistics
        for (int run = 0; run < numRuns; run++) {
            vector<Point> points = generateRandom2D(n);

            auto start = high_resolution_clock::now();
            auto hull = hullFunc(points);
            auto end = high_resolution_clock::now();

            // Time in microseconds
            double time_us = duration_cast<microseconds>(end - start).count();
            // Approximate memory in KB
            double mem_kb = (sizeof(Point) * points.size() +
                             sizeof(Point) * hull.size()) / 1024.0;

            times.push_back(time_us);
            mems.push_back(mem_kb);

            // Save generated points and computed hull
            savePoints(points, "results/" + algName + "/" + algName +
                       "_points_" + to_string(n) + "_" + to_string(run+1) + ".txt");
            savePoints(hull, "results/" + algName + "/" + algName +
                       "_hull_" + to_string(n) + "_" + to_string(run+1) + ".txt");
        }

        // Compute average and standard deviation
        double avg_time = computeMean(times);
        double avg_mem  = computeMean(mems);
        double std_time = computeStd(times, avg_time);
        double std_mem  = computeStd(mems, avg_mem);

        // Log results to console
        cout << "n=" << n << " avg_time=" << avg_time << " us ±" << std_time
             << ", avg_mem=" << avg_mem << " KB ±" << std_mem << "\n";

        // Log results to CSV
        out << n << "," << avg_time << "," << std_time
            << "," << avg_mem << "," << std_mem << "\n";
    }

    out.close();
}

// ------------------ 3D Experiment ------------------
template<typename Func>
void Evaluation::evaluate3D(Func hullFunc,
                            const vector<int>& sizes,
                            const string& algName) {
    fs::create_directories("results/" + algName);
    ofstream out("results/" + algName + "/" + algName + "_results.csv");
    out << "n,avg_time_us,std_time_us,avg_mem_KB,std_mem_KB\n";

    for (int n : sizes) {
        int runs = numRuns;
        bool singleRun = false;

        // Large QuickHull3D experiments use single run to save time
        if (algName == "quickHull3D" && n >= 20000) {
            runs = 1;
            singleRun = true;
        }

        vector<double> times, mems;

        for (int run = 0; run < runs; run++) {
            // Generate points
            vector<Point3> points = (algName == "quickHull3D")
                ? generateRandomSphere3D(n, 1.0)
                : generateRandom3D(n);

            auto start = high_resolution_clock::now();
            auto hullOutput = hullFunc(points);
            auto end = high_resolution_clock::now();

            double time_us = duration_cast<microseconds>(end - start).count();

            // Estimate memory usage
            size_t mem_bytes = sizeof(Point3) * points.size();
            if constexpr (is_same_v<decltype(hullOutput), Hull3D>)
                mem_bytes += sizeof(Point3) * hullOutput.vertices.size();
            else
                mem_bytes += sizeof(Point3) * hullOutput.size();

            double mem_kb = mem_bytes / 1024.0;

            times.push_back(time_us);
            mems.push_back(mem_kb);

            // Save points
            savePoints(points, "results/" + algName + "/" + algName +
                       "_points_" + to_string(n) + "_" + to_string(run+1) + ".txt");

            // Save hull depending on type
            if constexpr (is_same_v<decltype(hullOutput), Hull3D>) {
                savePoints(hullOutput.vertices, "results/" + algName + "/" + algName +
                           "_vertices_" + to_string(n) + "_" + to_string(run+1) + ".txt");
                ofstream fout("results/" + algName + "/" + algName +
                              "_faces_" + to_string(n) + "_" + to_string(run+1) + ".txt");
                for (auto &f : hullOutput.faces)
                    fout << f[0] << " " << f[1] << " " << f[2] << "\n";
                fout.close();
            } else {
                savePoints(hullOutput, "results/" + algName + "/" + algName +
                           "_hull_" + to_string(n) + "_" + to_string(run+1) + ".txt");
            }
        }

        // Compute averages and standard deviations
        double avg_time = computeMean(times);
        double avg_mem  = computeMean(mems);
        double std_time = singleRun ? 0.0 : computeStd(times, avg_time);
        double std_mem  = singleRun ? 0.0 : computeStd(mems, avg_mem);

        // Console logging
        if (!singleRun) {
            cout << "n=" << n << " avg_time=" << avg_time << " us ±" << std_time
                 << ", avg_mem=" << avg_mem << " KB ±" << std_mem << "\n";
        } else {
            cout << "n=" << n << " time=" << avg_time
                 << " us, mem=" << avg_mem << " KB\n";
        }

        // CSV logging
        out << n << "," << avg_time << "," << std_time
            << "," << avg_mem << "," << std_mem << "\n";
    }

    out.close();
}

#endif // EVALUATION_TPP
