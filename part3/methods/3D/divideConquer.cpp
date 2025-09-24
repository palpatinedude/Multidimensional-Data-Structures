/*
 * divideConquer3D.cpp
 * -------------------
 * Implements the 3D Divide & Conquer convex hull algorithm.
 *
 * Concept:
 * -----------
 * Convex hull in 3D is the smallest convex polyhedron containing all points.
 * Divide & Conquer works as follows:
 *
 * 1. For small point sets (≤5 points), compute the hull using brute-force:
 *    - Consider all triplets of points to form candidate faces.
 *    - A face is valid if all other points lie on one side of its plane.
 * 2. For larger sets:
 *    - Sort points (by x, then y, then z).
 *    - Divide into left and right halves.
 *    - Recursively compute convex hulls for left and right halves.
 *    - Merge the two hulls:
 *      - Consider all triplets from both hulls as potential faces.
 *      - Keep only faces that are not visible from any other point.
 *
 * -----------------
 * - Floating-point precision is critical, dot products and cross products use doubles.
 * - Face::visible` uses dot product to check if a point lies in front of the face plane.
 * - Resulting hull is returned as a set of vertices (faces are not stored in final output).
 */

#include "../../common.h"
#include <set>
#include <tuple>
#include <algorithm>
using namespace std;


// ---------------------- 3D Face Structure ----------------------
struct Face {
    Point3 a, b, c;
    tuple<double,double,double> normal;

    // Check if a point lies "above" the face
    bool visible(const Point3 &p) const {
        double nx, ny, nz; tie(nx, ny, nz) = normal;
        double vx = p.x - a.x, vy = p.y - a.y, vz = p.z - a.z;
        return nx*vx + ny*vy + nz*vz > 0; // dot product
    }
};

// ---------------------- Brute-force hull for small 3D sets ----------------------
vector<Point3> bruteHull3D(const vector<Point3> &pts){
    int n = pts.size();
    set<Point3> vertices;
    if(n == 0) return {};
    if(n <= 3) { for(auto &p : pts) vertices.insert(p); return vector<Point3>(vertices.begin(), vertices.end()); }

    // Check all triplets for candidate faces
    for(int i=0; i<n; i++)
        for(int j=i+1; j<n; j++)
            for(int k=j+1; k<n; k++){
                auto nvec = cross3(pts[i], pts[j], pts[k]);
                double nx, ny, nz; tie(nx, ny, nz) = nvec;
                if(nx==0 && ny==0 && nz==0) continue; // degenerate plane
                int pos=0, neg=0;
                for(int t=0; t<n; t++){
                    double d = dotWith(nvec, pts[i], pts[t]);
                    if(d > 0) pos++;
                    else if(d < 0) neg++;
                }
                if(pos==0 || neg==0){ // all points on one side
                    vertices.insert(pts[i]);
                    vertices.insert(pts[j]);
                    vertices.insert(pts[k]);
                }
            }
    return vector<Point3>(vertices.begin(), vertices.end());
}

// ---------------------- Merge two 3D hulls ----------------------
vector<Point3> mergeHulls3D(const vector<Point3> &leftVerts, const vector<Point3> &rightVerts){
    vector<Face> faces;

    // Helper: add all possible faces from a set of vertices
    auto addFaces = [&](const vector<Point3> &verts){
        int n = verts.size();
        for(int i=0; i<n; i++)
            for(int j=i+1; j<n; j++)
                for(int k=j+1; k<n; k++){
                    auto nvec = cross3(verts[i], verts[j], verts[k]);
                    double nx, ny, nz; tie(nx, ny, nz) = nvec;
                    if(nx==0 && ny==0 && nz==0) continue; // degenerate
                    faces.push_back({verts[i], verts[j], verts[k], nvec});
                }
    };
    addFaces(leftVerts);
    addFaces(rightVerts);

    // Combine vertices from both hulls
    vector<Point3> combined = leftVerts;
    combined.insert(combined.end(), rightVerts.begin(), rightVerts.end());

    // Filter faces: keep only those not visible from any point
    vector<Face> finalFaces;
    for(auto &f : faces){
        bool visibleFlag = false;
        for(auto &p : combined){
            if(f.visible(p)) { visibleFlag = true; break; }
        }
        if(!visibleFlag) finalFaces.push_back(f);
    }

    // Collect unique hull vertices
    set<Point3> hullVerts;
    for(auto &f : finalFaces){
        hullVerts.insert(f.a);
        hullVerts.insert(f.b);
        hullVerts.insert(f.c);
    }

    return vector<Point3>(hullVerts.begin(), hullVerts.end());
}

// ---------------------- Main 3D Divide & Conquer ----------------------
vector<Point3> divideAndConquer3D(vector<Point3>& points){
    int n = points.size();
    if(n <= 5) return bruteHull3D(points); // small set: brute-force

    sort(points.begin(), points.end()); // sort by x, then y, then z
    vector<Point3> left(points.begin(), points.begin()+n/2);
    vector<Point3> right(points.begin()+n/2, points.end());

    vector<Point3> leftHull = divideAndConquer3D(left);   // recursive left
    vector<Point3> rightHull = divideAndConquer3D(right); // recursive right

    return mergeHulls3D(leftHull, rightHull); // merge two hulls
}
