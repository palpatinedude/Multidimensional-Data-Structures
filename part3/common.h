/*
 * common.h
 * ----------
 * This header provides basic structures and utility functions for 2D and 3D geometry:
 * - 2D Point (Point) and 3D Point (Point3) with comparison operators.
 * - 2D geometric helpers: orientation, counter-clockwise check (ccw).
 * - 3D geometric helpers: cross product, dot product for face visibility.
 * - Random point generation in 2D and 3D.
 * - File saving for points to text files.
 * - These are building blocks for convex hull algorithms.
 */

#ifndef COMMON_H
#define COMMON_H

#include <bits/stdc++.h>
using namespace std;


// ---------------------- 2D Point ----------------------
struct Point {
    double x, y;

    // Comparison for sorting: first by x, then by y
    bool operator<(const Point& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }

    // Equality with tolerance for floating-point errors
    bool operator==(const Point& other) const {
        return fabs(x - other.x) < 1e-9 && fabs(y - other.y) < 1e-9;
    }
};

// ---------------------- 3D Point ----------------------
struct Point3 {
    double x, y, z;

    // Lexicographical comparison with epsilon tolerance
    bool operator<(const Point3& other) const {
        if (fabs(x - other.x) > 1e-9) return x < other.x;
        if (fabs(y - other.y) > 1e-9) return y < other.y;
        return z < other.z;
    }

    // Equality with tolerance for floating-point errors
    bool operator==(const Point3& other) const {
        return fabs(x - other.x) < 1e-9 &&
               fabs(y - other.y) < 1e-9 &&
               fabs(z - other.z) < 1e-9;
    }
};

// ---------------------- 2D Geometry ----------------------
// Returns orientation of triplet (a,b,c): 0 = collinear, 1 = clockwise, -1 = counter-clockwise
int orientation(const Point &a, const Point &b, const Point &c);

// Returns true if points a,b,c are in counter-clockwise order
bool ccw(const Point &a, const Point &b, const Point &c);

// ---------------------- 3D Geometry ----------------------
// Cross product of vectors AB and AC for 3D points, returns normal vector (tuple)
tuple<double,double,double> cross3(const Point3 &a, const Point3 &b, const Point3 &c);

// Dot product of vector from a to p with normal vector n
double dotWith(const tuple<double,double,double> &n, const Point3 &a, const Point3 &p);

// ---------------------- Random Point Generation ----------------------
// Generate n random 2D points within [minCoord, maxCoord]
vector<Point> generateRandom2D(int n, double minCoord=-1e5, double maxCoord=1e5);

// Generate n random 3D points within [minCoord, maxCoord]
vector<Point3> generateRandom3D(int n, double minCoord=-1e5, double maxCoord=1e5);

// Generate n random 3D points on the surface of a sphere 
vector<Point3> generateRandomSphere3D(int n, double radius=1.0);

// ---------------------- File Saving ----------------------
// Save 2D points to a text file (x y per line)
void savePoints(const vector<Point> &pts, const string &filename);

// Save 3D points to a text file (x y z per line)
void savePoints(const vector<Point3> &pts, const string &filename);

#endif
