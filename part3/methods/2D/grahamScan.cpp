/*
grahamScan2D.cpp
----------------
Purpose:
- Implements the Graham Scan algorithm to compute the 2D convex hull of a set of points.
- Returns the convex hull in counter-clockwise order and includes all boundary points (even collinear ones).

* Concept:
*-----------
- Steps:
  1. Identify the point with the lowest y-coordinate (and leftmost x-coordinate if tie). This is the starting point (p0).
  2. Sort all points by the polar angle they make with p0.
     - If points are collinear, sort by distance from p0 (closer points first).
  3. Traverse the sorted points and maintain a "stack" of hull points.
     - For each point, remove the last point from the hull if it would create a clockwise turn.
     - Push the current point onto the hull.
  4. At the end, the stack contains the convex hull vertices in order.

*/

#include "../../common.h"
#include <vector>
#include <algorithm>
using namespace std;


// Check if three points form a clockwise turn
bool cw(const Point &a, const Point &b, const Point &c, bool include_collinear = false) {
    return !ccw(a, b, c) || (include_collinear && orientation(a, b, c) == 0);
}

// Squared distance between two points (used to order collinear points)
double distSq(const Point &a, const Point &b) {
    return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

// ---------------------- Graham Scan Convex Hull ----------------------
vector<Point> grahamScan2D(vector<Point>& points) {
    int n = points.size();
    if (n < 3) return points; // trivial hull for fewer than 3 points

    // Find the starting point p0 (lowest y, then lowest x if tie)
    Point p0 = *min_element(points.begin(), points.end(), [](const Point &a, const Point &b){
        return make_pair(a.y, a.x) < make_pair(b.y, b.x);
    });

    // Sort points by polar angle relative to p0
    sort(points.begin(), points.end(), [&p0](const Point &a, const Point &b){
        int o = orientation(p0, a, b);
        if (o == 0) return distSq(p0, a) < distSq(p0, b); // closer first if collinear
        return o < 0; // counter-clockwise points come first
    });

    // Reverse order of last collinear points to include all boundary points
    int i = n - 1;
    while (i >= 0 && orientation(p0, points[i], points.back()) == 0) i--;
    reverse(points.begin() + i + 1, points.end());

    // Build the convex hull
    vector<Point> hull;
    for (auto &pt : points) {
        // Remove last point if it would create a clockwise turn
        while (hull.size() > 1 && !cw(hull[hull.size()-2], hull.back(), pt, true)) {
            hull.pop_back();
        }
        hull.push_back(pt); // add current point
    }

    // Remove duplicate last point if only two points are equal
    if (hull.size() == 2 && hull[0] == hull[1])
        hull.pop_back();

    return hull;
}
