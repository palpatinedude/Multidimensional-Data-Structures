/*
 * allAlgorithms.h
 * ----------------
 * Declares the convex hull algorithms implemented in this project:
 * 1. 2D Divide & Conquer
 * 2. 3D Divide & Conquer
 * 3. 2D Monotone Chain
 * 4. 2D QuickHull
 * 5. 3D QuickHull
 * 6. 2D Graham Scan 

 */

#ifndef ALL_ALGORITHMS_H
#define ALL_ALGORITHMS_H

#include "common.h"
#include <vector>
using namespace std;

// ---------------------- 2D Divide & Conquer ----------------------
vector<Point> divideAndConquer2D(vector<Point>& points);

// ---------------------- 3D Divide & Conquer ----------------------
vector<Point3> divideAndConquer3D(vector<Point3>& points);

// ---------------------- 2D Monotone Chain ----------------------
vector<Point> monotoneChainHull(vector<Point>& pts);

// ---------------------- 2D QuickHull ----------------------
vector<Point> quickHull2D(vector<Point>& pts);

// ---------------------- 2D Graham Scan ----------------------
vector<Point> grahamScan2D(vector<Point>& points);

// ---------------------- 3D QuickHull ----------------------
struct Hull3D {
    vector<Point3> vertices;
    vector<array<int,3>> faces;
};

Hull3D quickHull3D(vector<Point3>& points);


#endif
