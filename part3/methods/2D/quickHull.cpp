/*
 * quickHull.cpp
 * --------------
 * Implements the 2D QuickHull convex hull algorithm.
 *
 * Concept:
 * ----------
 * QuickHull is a divide and conquer style convex hull algorithm.
 * Steps:
 * 1. Find extreme points with minimum and maximum x-coordinates (these form a baseline).
 * 2. Partition points into two subsets: above and below the line formed by the baseline.
 * 3. Recursively:
 *    - Find the farthest point from the current line segment.
 *    - This point becomes part of the hull.
 *    - Partition points relative to the new edges formed.
 * 4. Stop recursion when no points lie outside the current hull segment.
 *

 * -----------------
 * - Uses global `set<Point> hull` for incremental hull storage.
 * - Uses orientation and distance to determine side and farthest point.
 * - Final points are ordered counter-clockwise using centroid and polar angles.
 * - Handles degenerate cases where fewer than 3 points exist.
 */

#include "../../common.h"
#include <cmath>
using namespace std;



// ---------------------- Global hull storage ----------------------
set<Point> hull;

// ---------------------- Determine side of point relative to line ----------------------
int findSide(const Point &p1, const Point &p2, const Point &p) {
    double val = orientation(p1, p2, p);
    if(val == 0) return 0;        // point on the line
    return (val > 0) ? 1 : -1;   // 1: left side, -1: right side
}

// ---------------------- Distance from point to line ----------------------
double lineDist(const Point &p1, const Point &p2, const Point &p) {
    return abs((p.y - p1.y) * (p2.x - p1.x) - (p2.y - p1.y) * (p.x - p1.x));
}

// ---------------------- Recursive QuickHull ----------------------
void quickHull(const vector<Point> &pts, const Point &p1, const Point &p2, int side) {
    int ind = -1;
    double maxDist = 0;

    // Find farthest point from line segment on specified side
    for (int i = 0; i < (int)pts.size(); i++) {
        double d = lineDist(p1, p2, pts[i]);
        if(findSide(p1, p2, pts[i]) == side && d > maxDist) {
            ind = i; maxDist = d;
        }
    }

    if (ind == -1) {
        // No points outside segment, add endpoints to hull
        hull.insert(p1);
        hull.insert(p2);
        return;
    }

    // Recurse on two segments formed with farthest point
    quickHull(pts, pts[ind], p1, -findSide(pts[ind], p1, p2));
    quickHull(pts, pts[ind], p2, -findSide(pts[ind], p2, p1));
}

// ---------------------- Order points CCW using centroid ----------------------
vector<Point> orderHullCCW(const set<Point> &hullSet) {
    vector<Point> pts(hullSet.begin(), hullSet.end());
    if (pts.empty()) return pts;

    double cx = 0, cy = 0;
    for(auto &p : pts) { cx += p.x; cy += p.y; }
    double centroidX = cx / pts.size();
    double centroidY = cy / pts.size();

    sort(pts.begin(), pts.end(), [&](const Point &a, const Point &b) {
        double angA = atan2(a.y - centroidY, a.x - centroidX);
        double angB = atan2(b.y - centroidY, b.x - centroidX);
        return angA < angB;
    });

    return pts;
}

// ---------------------- QuickHull main function ----------------------
vector<Point> quickHull2D(vector<Point>& pts) {
    hull.clear();
    if (pts.size() < 3) return pts; // trivial hull

    // Find leftmost and rightmost points
    int min_x = 0, max_x = 0;
    for (int i = 1; i < (int)pts.size(); i++) {
        if (pts[i].x < pts[min_x].x) min_x = i;
        if (pts[i].x > pts[max_x].x) max_x = i;
    }

    // Recurse for points above and below baseline
    quickHull(pts, pts[min_x], pts[max_x], 1);
    quickHull(pts, pts[min_x], pts[max_x], -1);

    return orderHullCCW(hull);
}


