/*
 * monotoneChain.cpp
 * -----------------
 * Implements the 2D Monotone Chain convex hull algorithm (also known as Andrew's algorithm).
 *
 * Concept:
 * ----------
 * Steps:
 * 1. Sort all points lexicographically by x, then y.
 * 2. Build lower hull:
 *    - Traverse points left to right.
 *    - Maintain a stack of hull points, removing the last point if the new point
 *      would create a clockwise turn (non-CCW).
 * 3. Build upper hull:
 *    - Traverse points right to left.
 *    - Apply the same process as lower hull.
 * 4. Concatenate lower and upper hulls (excluding the last point of each to avoid duplication).
 
 * -----------------
 * - Uses a double-based cross product for precision.
 * - Handles degenerate cases: if n <= 3, the input points themselves form the hull.
 * - Output points are in counter-clockwise order.
 */


#include "../../common.h"
using namespace std;


// ---------------------- 2D Cross Product Helper ----------------------
double crossProduct(const Point &O, const Point &A, const Point &B) {
    // Returns the z-component of (OA × OB)
    // Positive -> counter-clockwise turn
    // Negative -> clockwise turn
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// ---------------------- Monotone Chain Convex Hull ----------------------
vector<Point> monotoneChainHull(vector<Point>& pts) {
    int n = pts.size(), k = 0;
    if (n <= 3) return pts; // trivial hull

    sort(pts.begin(), pts.end()); // lexicographical sort by x, then y
    vector<Point> hull(2 * n);   // temporary space for lower + upper hull

    // Build lower hull
    for (int i = 0; i < n; i++) {
        while (k >= 2 && crossProduct(hull[k-2], hull[k-1], pts[i]) <= 0.0) k--;
        hull[k++] = pts[i];
    }

    // Build upper hull
    for (int i = n - 2, t = k + 1; i >= 0; i--) {
        while (k >= t && crossProduct(hull[k-2], hull[k-1], pts[i]) <= 0.0) k--;
        hull[k++] = pts[i];
    }

    hull.resize(k - 1); // remove duplicate endpoint
    return hull;
}

