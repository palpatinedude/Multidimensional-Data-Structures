/*
 * common.cpp
 * ----------
 * Implements basic 2D and 3D geometry utilities, random point generation,
 * and functions to save points to files.
 *
 * Key points:
 * - 2D geometry: orientation and counter-clockwise check (ccw).
 * - 3D geometry: cross product and dot product for face visibility.
 * - Random points use uniform distribution in specified range.
 * - File saving writes points in text format (x y) or (x y z).
 */


#include "common.h"

// ---------------------- 2D Geometry ----------------------

// Returns orientation of triplet (a,b,c):
// 0 = collinear, 1 = clockwise, -1 = counter-clockwise
int orientation(const Point &a, const Point &b, const Point &c) {
    double val = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    if(fabs(val) < 1e-9) return 0;
    return (val > 0) ? 1 : -1;
}

// Returns true if points a,b,c are in counter-clockwise order
bool ccw(const Point &a, const Point &b, const Point &c) {
    return orientation(a, b, c) > 0;
}

// ---------------------- 3D Geometry ----------------------

// Cross product of vectors AB and AC (a->b x a->c), returns normal vector
tuple<double,double,double> cross3(const Point3 &a, const Point3 &b, const Point3 &c) {
    double ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
    double vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
    double cx = uy*vz - uz*vy;
    double cy = uz*vx - ux*vz;
    double cz = ux*vy - uy*vx;
    return {cx, cy, cz};
}

// Dot product of vector from a to p with normal vector n
double dotWith(const tuple<double,double,double> &n, const Point3 &a, const Point3 &p) {
    double nx, ny, nz;
    tie(nx, ny, nz) = n;
    double vx = p.x - a.x, vy = p.y - a.y, vz = p.z - a.z;
    return nx*vx + ny*vy + nz*vz;
}

// ---------------------- Random Point Generation ----------------------

// Generate n random 2D points within [minCoord, maxCoord]
vector<Point> generateRandom2D(int n, double minCoord, double maxCoord) {
    vector<Point> points;
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> dist(minCoord, maxCoord);
    for(int i=0;i<n;i++) points.push_back({dist(rng), dist(rng)});
    return points;
}

// Generate n random 3D points within [minCoord, maxCoord]
vector<Point3> generateRandom3D(int n, double minCoord, double maxCoord) {
    vector<Point3> points;
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> dist(minCoord, maxCoord);
    for(int i=0;i<n;i++) points.push_back({dist(rng), dist(rng), dist(rng)});
    return points;
}

// Generate n random 3D points uniformly distributed inside a sphere
vector<Point3> generateRandomSphere3D(int n, double radius) {
    vector<Point3> points;
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> u(-1.0, 1.0);
    for(int i=0;i<n;i++){
        double x, y, z;
        while(true){
            x = u(rng); y = u(rng); z = u(rng);
            if(x*x + y*y + z*z <= 1.0) break; // inside unit sphere
        }
        // scale to desired radius
        points.push_back({x*radius, y*radius, z*radius});
    }
    return points;
}


// ---------------------- File Saving ----------------------

// Save 2D points to a text file (x y per line)
void savePoints(const vector<Point> &pts, const string &filename) {
    ofstream fout(filename);
    for(auto &p : pts) fout << p.x << " " << p.y << "\n";
    fout.close();
}

// Save 3D points to a text file (x y z per line)
void savePoints(const vector<Point3> &pts, const string &filename) {
    ofstream fout(filename);
    for(auto &p : pts) fout << p.x << " " << p.y << " " << p.z << "\n";
    fout.close();
}
