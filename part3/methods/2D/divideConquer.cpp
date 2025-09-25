/*
 * divideConquer2D.cpp
 * -------------------
 * Implements the 2D Divide & Conquer convex hull algorithm.
 *
 * Concept:
 * -----------
 * Convex hull of a set of points is the smallest convex polygon that
 * contains all points. Divide & Conquer builds the hull efficiently:
 *
 * 1. If the set is small (<=5 points), compute hull using brute-force.
 *    - Check all pairs of points and see if all other points lie on
 *      one side of the line segment connecting the pair.
 * 2. For larger sets:
 *    - Sort points by x-coordinate (and y to break ties).
 *    - Divide the set into two halves (left and right).
 *    - Recursively compute the convex hull of each half.
 *    - Merge the two hulls using a Graham-scan-like approach:
 *      - Pick a pivot (lowest y-coordinate, tie lowest x).
 *      - Sort points around the pivot by polar angle.
 *      - Scan to remove points that are not part of the final hull.
 *

 * -----------------
 * - Global variable `centroid` is used in brute-force hull for CCW sorting.
 * - Floating-point comparisons use small epsilon (1e-9) to avoid precision issues.
 * - Merge function ensures strict weak ordering for sorting.
 */

#include "../../common.h"
#include <algorithm>
#include <vector>
#include <cmath>
using namespace std;


// ---------------------- Global centroid for CCW sorting ----------------------
pair<double,double> centroid;

// ---------------------- Helper: Quadrant of a point relative to centroid ----------------------
int quadrant(pair<double,double> p) {
    if(p.first >= 0 && p.second >= 0) return 1;
    if(p.first <= 0 && p.second >= 0) return 2;
    if(p.first <= 0 && p.second <= 0) return 3;
    return 4;
}

// ---------------------- Helper: CCW comparison around centroid ----------------------
bool ccwCompare(const Point &a, const Point &b) {
    pair<double,double> pa = {a.x - centroid.first, a.y - centroid.second};
    pair<double,double> pb = {b.x - centroid.first, b.y - centroid.second};
    int quadA = quadrant(pa), quadB = quadrant(pb);

    if(quadA != quadB) return quadA < quadB; // points in different quadrants
    return pa.second * pb.first < pb.second * pa.first; // slope comparison
}

// ---------------------- Brute-force hull for small point sets ----------------------
vector<Point> bruteHull(vector<Point> pts) {
    int n = pts.size();
    set<Point> hullSet;

    // Check all pairs of points to see if the line segment between them has all points on one side
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double A = pts[i].y - pts[j].y;
            double B = pts[j].x - pts[i].x;
            double C = pts[i].x * pts[j].y - pts[i].y * pts[j].x;
            int pos = 0, neg = 0;

            for (int k = 0; k < n; ++k) {
                double val = A * pts[k].x + B * pts[k].y + C;
                if (val > 1e-9) pos++;
                else if (val < -1e-9) neg++;
            }

            if (pos == 0 || neg == 0) { // all points on one side of line
                hullSet.insert(pts[i]);
                hullSet.insert(pts[j]);
            }
        }
    }

    vector<Point> hull(hullSet.begin(), hullSet.end());
    int m = hull.size();
    if (m == 0) return hull;

    // Compute centroid for CCW sorting
    centroid = {0.0, 0.0};
    for (auto& p : hull) {
        centroid.first += p.x;
        centroid.second += p.y;
    }
    centroid.first /= m;
    centroid.second /= m;

    // Sort points counter-clockwise around centroid
    sort(hull.begin(), hull.end(), ccwCompare);

    return hull;
}

// ---------------------- Merge two convex hulls ----------------------
vector<Point> mergeHulls(const vector<Point> &left, const vector<Point> &right) {
    vector<Point> combined = left;
    combined.insert(combined.end(), right.begin(), right.end());
    if (combined.empty()) return {};

    // Choose pivot: lowest y (tie-break with lowest x)
    Point pivot = *min_element(combined.begin(), combined.end(), [](const Point &a, const Point &b){
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    });

    // Sort points around pivot for Graham scan
    sort(combined.begin(), combined.end(), [&](const Point &a, const Point &b){
        if (a == b) return false;
        if (a == pivot && b == pivot) return false;
        if (a == pivot) return true;
        if (b == pivot) return false;

        int o = orientation(pivot, a, b);
        if (o == 0) { // collinear, use distance to pivot
            double da = (a.x - pivot.x)*(a.x - pivot.x) + (a.y - pivot.y)*(a.y - pivot.y);
            double db = (b.x - pivot.x)*(b.x - pivot.x) + (b.y - pivot.y)*(b.y - pivot.y);
            if (fabs(da - db) < 1e-12) return a < b;
            return da < db;
        }
        return o > 0; // CCW ordering
    });

    // Graham scan: remove points that are not part of convex hull
    vector<Point> hull;
    hull.reserve(combined.size());
    for (const auto &p : combined) {
        while (hull.size() >= 2 && orientation(hull[hull.size()-2], hull.back(), p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    return hull;
}

// ---------------------- Main Divide & Conquer Function ----------------------
vector<Point> divideAndConquer2D(vector<Point>& points) {
    int n = points.size();
    if(n <= 5) return bruteHull(points); // small set: brute-force

    sort(points.begin(), points.end()); // sort by x (tie y)
    vector<Point> left(points.begin(), points.begin()+n/2);
    vector<Point> right(points.begin()+n/2, points.end());

    vector<Point> leftHull = divideAndConquer2D(left);   // recursive left half
    vector<Point> rightHull = divideAndConquer2D(right); // recursive right half

    return mergeHulls(leftHull, rightHull); // merge two hulls
}
