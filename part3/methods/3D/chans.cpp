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


/*
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
*/
/*
#include "../../common.h"
#include "../../allAlgorithms.h"
#include <set>
#include <tuple>
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>
#include <map>
using namespace std;

// ---------------------- 3D Face Structure ----------------------
struct Face {
    Point3 a, b, c;
    tuple<double,double,double> normal;
    bool visibleFlag = false;

    Face(Point3 a_, Point3 b_, Point3 c_) : a(a_), b(b_), c(c_) {
        normal = cross3(a, b, c);
    }

    bool visible(const Point3 &p) const {
        const double EPS = 1e-9;
        double nx, ny, nz; tie(nx, ny, nz) = normal;
        double vx = p.x - a.x, vy = p.y - a.y, vz = p.z - a.z;
        return nx*vx + ny*vy + nz*vz > EPS;
    }
};

// ---------------------- Brute-force hull for very small sets ----------------------
vector<Point3> bruteHull3D(const vector<Point3> &pts){
    set<Point3> vertices;
    if(pts.size() <= 3) { 
        for(auto &p : pts) vertices.insert(p); 
        return vector<Point3>(vertices.begin(), vertices.end()); 
    }

    for(size_t i=0;i<pts.size();i++)
        for(size_t j=i+1;j<pts.size();j++)
            for(size_t k=j+1;k<pts.size();k++){
                auto nvec = cross3(pts[i], pts[j], pts[k]);
                double nx, ny, nz; tie(nx, ny, nz) = nvec;
                const double EPS = 1e-9;
                if(fabs(nx)<EPS && fabs(ny)<EPS && fabs(nz)<EPS) continue;
                int pos=0, neg=0;
                for(size_t t=0; t<pts.size(); t++){
                    double d = dotWith(nvec, pts[i], pts[t]);
                    if(d > EPS) pos++;
                    else if(d < -EPS) neg++;
                }
                if(pos==0 || neg==0){
                    vertices.insert(pts[i]);
                    vertices.insert(pts[j]);
                    vertices.insert(pts[k]);
                }
            }
    return vector<Point3>(vertices.begin(), vertices.end());
}

// ---------------------- Build initial tetrahedron ----------------------
vector<Face*> buildInitialHull(const vector<Point3>& pts){
    vector<Face*> faces;
    if(pts.size() < 4) return faces;

    for(size_t i=0;i<pts.size();i++)
      for(size_t j=i+1;j<pts.size();j++)
        for(size_t k=j+1;k<pts.size();k++)
          for(size_t l=k+1;l<pts.size();l++){
              auto nvec = cross3(pts[i], pts[j], pts[k]);
              if(fabs(dotWith(nvec, pts[i], pts[l])) > 1e-9){
                  faces.push_back(new Face(pts[i], pts[j], pts[k]));
                  faces.push_back(new Face(pts[i], pts[k], pts[l]));
                  faces.push_back(new Face(pts[i], pts[l], pts[j]));
                  faces.push_back(new Face(pts[j], pts[l], pts[k]));
                  return faces;
              }
          }
    return faces;
}

// ---------------------- Incremental addition using horizon ----------------------
void addPointToHull(Point3 p, vector<Face*>& hull){
    vector<Face*> visibleFaces;
    for(auto f : hull){
        if(f->visible(p)) f->visibleFlag = true, visibleFaces.push_back(f);
        else f->visibleFlag = false;
    }

    set<pair<Point3,Point3>> horizon;
    for(auto f : visibleFaces){
        pair<Point3,Point3> edges[3] = {{f->a,f->b},{f->b,f->c},{f->c,f->a}};
        for(auto &e : edges){
            bool sharedWithVisible = false;
            for(auto f2 : visibleFaces){
                if(f2 == f) continue;
                if((f2->a==e.second && f2->b==e.first) ||
                   (f2->b==e.second && f2->c==e.first) ||
                   (f2->c==e.second && f2->a==e.first)){
                    sharedWithVisible = true; break;
                }
            }
            if(!sharedWithVisible) horizon.insert(e);
        }
    }

    hull.erase(remove_if(hull.begin(), hull.end(),
                         [](Face* f){ return f->visibleFlag; }), hull.end());

    for(auto &[u,v] : horizon){
        hull.push_back(new Face(u,v,p));
    }
}

// ---------------------- Merge two hulls ----------------------
Hull3D mergeHulls3D(const Hull3D &hullA, const Hull3D &hullB){
    vector<Point3> combined = hullA.vertices;
    combined.insert(combined.end(), hullB.vertices.begin(), hullB.vertices.end());

    vector<Face*> faces = buildInitialHull(combined);

    for(auto &p : combined) addPointToHull(p, faces);

    // Extract unique vertices and remap faces
    vector<Point3> vertices;
    map<Point3,int> idxMap;
    for(auto f : faces){
        for(auto pt : {f->a, f->b, f->c}){
            if(idxMap.find(pt) == idxMap.end()){
                idxMap[pt] = vertices.size();
                vertices.push_back(pt);
            }
        }
    }

    vector<array<int,3>> outFaces;
    for(auto f : faces){
        outFaces.push_back({idxMap[f->a], idxMap[f->b], idxMap[f->c]});
        delete f;
    }

    return {vertices, outFaces};
}

// ---------------------- Chan's 3D Convex Hull ----------------------
Hull3D chan3DConvexHull(vector<Point3>& points){
    size_t n = points.size();
    if(n <= 50){
        vector<Point3> hullPts = bruteHull3D(points);
        return {hullPts, {}}; // faces empty for small sets
    }

    size_t m = 32;
    Hull3D hull;

    while(true){
        vector<vector<Point3>> subsets;
        for(size_t i=0;i<n;i+=m){
            size_t end = min(i+m, n);
            subsets.push_back(vector<Point3>(points.begin()+i, points.begin()+end));
        }

        vector<Hull3D> subsetHulls;
        for(auto &s : subsets){
            if(s.size() <= 50) subsetHulls.push_back({bruteHull3D(s), {}});
            else subsetHulls.push_back(chan3DConvexHull(s));
        }

        hull = subsetHulls[0];
        for(size_t i=1;i<subsetHulls.size();i++)
            hull = mergeHulls3D(hull, subsetHulls[i]);

        if(hull.vertices.size() <= m) break;
        m *= 2;
    }

    return hull;
}*/

#include "../../common.h"
#include "../../allAlgorithms.h"
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

// ---------------------- 3D Face Structure ----------------------
struct Face {
    Point3 a, b, c;
    tuple<double,double,double> normal;
    bool visibleFlag = false;

    Face(Point3 a_, Point3 b_, Point3 c_) : a(a_), b(b_), c(c_) {
        normal = cross3(a, b, c);
    }

    bool visible(const Point3 &p) const {
        const double EPS = 1e-9;
        double nx, ny, nz; tie(nx, ny, nz) = normal;
        double vx = p.x - a.x, vy = p.y - a.y, vz = p.z - a.z;
        return nx*vx + ny*vy + nz*vz > EPS;
    }

    array<pair<Point3,Point3>,3> edges() const {
        return {{{a,b},{b,c},{c,a}}};
    }
};

// ---------------------- Build initial tetrahedron ----------------------
vector<Face*> buildInitialTetrahedron(const vector<Point3>& pts) {
    vector<Face*> hull;
    size_t n = pts.size();
    for(size_t i=0;i<n;i++)
        for(size_t j=i+1;j<n;j++)
            for(size_t k=j+1;k<n;k++)
                for(size_t l=k+1;l<n;l++){
                    auto nvec = cross3(pts[i], pts[j], pts[k]);
                    if(fabs(dotWith(nvec, pts[i], pts[l])) > 1e-9){
                        hull.push_back(new Face(pts[i], pts[j], pts[k]));
                        hull.push_back(new Face(pts[i], pts[k], pts[l]));
                        hull.push_back(new Face(pts[i], pts[l], pts[j]));
                        hull.push_back(new Face(pts[j], pts[l], pts[k]));
                        return hull;
                    }
                }
    return hull;
}

// ---------------------- Add point using horizon ----------------------
void addPointToHull(Point3 p, vector<Face*>& hull){
    vector<Face*> visibleFaces;
    for(auto f : hull) if(f->visible(p)) f->visibleFlag = true, visibleFaces.push_back(f);
    if(visibleFaces.empty()) return;

    map<pair<Point3,Point3>,int> edgeCount;
    for(auto f : visibleFaces){
        for(auto [u,v] : f->edges()){
            edgeCount[{u,v}]++;
            edgeCount[{v,u}]--; // reverse edge
        }
    }

    // Remove visible faces
    hull.erase(remove_if(hull.begin(), hull.end(),
                         [](Face* f){ return f->visibleFlag; }), hull.end());
    for(auto f : visibleFaces) delete f;

    // Add new faces from horizon edges
    for(auto &[e,c] : edgeCount){
        if(c==1){
            hull.push_back(new Face(e.first, e.second, p));
        }
    }
}

// ---------------------- Incremental hull with faces ----------------------
vector<Face*> incrementalHull3DWithFaces(vector<Point3>& pts){
    vector<Face*> hull = buildInitialTetrahedron(pts);
    if(hull.empty()) return hull;
    for(auto &p : pts) addPointToHull(p, hull);
    return hull;
}

// ---------------------- Merge two hulls ----------------------
Hull3D mergeHulls3D(const Hull3D &hA, const Hull3D &hB){
    vector<Point3> combined = hA.vertices;
    combined.insert(combined.end(), hB.vertices.begin(), hB.vertices.end());
    vector<Face*> faces = incrementalHull3DWithFaces(combined);

    vector<Point3> vertices;
    map<Point3,int> idxMap;
    vector<array<int,3>> outFaces;

    for(auto f : faces){
        for(auto pt : {f->a,f->b,f->c}){
            if(idxMap.find(pt)==idxMap.end()){
                idxMap[pt] = vertices.size();
                vertices.push_back(pt);
            }
        }
    }

    for(auto f : faces){
        outFaces.push_back({idxMap[f->a], idxMap[f->b], idxMap[f->c]});
        delete f;
    }

    return {vertices, outFaces};
}

// ---------------------- Chan's 3D Convex Hull (QuickHull3D-compatible output) ----------------------
Hull3D chan3DConvexHull(vector<Point3>& points){
    size_t n = points.size();
    if(n <= 50){
        vector<Face*> faces = incrementalHull3DWithFaces(points);
        vector<Point3> vertices;
        map<Point3,int> idxMap;
        vector<array<int,3>> outFaces;
        for(auto f : faces){
            for(auto pt : {f->a,f->b,f->c}){
                if(idxMap.find(pt)==idxMap.end()){
                    idxMap[pt] = vertices.size();
                    vertices.push_back(pt);
                }
            }
        }
        for(auto f : faces){
            outFaces.push_back({idxMap[f->a], idxMap[f->b], idxMap[f->c]});
            delete f;
        }
        return {vertices, outFaces};
    }

    size_t m = 64;
    Hull3D hull;

    while(true){
        vector<vector<Point3>> subsets;
        for(size_t i=0;i<n;i+=m){
            size_t end = min(i+m,n);
            subsets.push_back(vector<Point3>(points.begin()+i, points.begin()+end));
        }

        vector<Hull3D> subsetHulls;
        for(auto &s : subsets){
            vector<Face*> faces = incrementalHull3DWithFaces(s);
            vector<Point3> vertices;
            map<Point3,int> idxMap;
            vector<array<int,3>> outFaces;
            for(auto f : faces){
                for(auto pt : {f->a,f->b,f->c}){
                    if(idxMap.find(pt)==idxMap.end()){
                        idxMap[pt] = vertices.size();
                        vertices.push_back(pt);
                    }
                }
            }
            for(auto f : faces){
                outFaces.push_back({idxMap[f->a], idxMap[f->b], idxMap[f->c]});
                delete f;
            }
            subsetHulls.push_back({vertices,outFaces});
        }

        hull = subsetHulls[0];
        for(size_t i=1;i<subsetHulls.size();i++)
            hull = mergeHulls3D(hull, subsetHulls[i]);

        if(hull.vertices.size() <= m) break;
        m *= 2;
        if(m > n){ // fallback
            vector<Face*> faces = incrementalHull3DWithFaces(points);
            vector<Point3> vertices;
            map<Point3,int> idxMap;
            vector<array<int,3>> outFaces;
            for(auto f : faces){
                for(auto pt : {f->a,f->b,f->c}){
                    if(idxMap.find(pt)==idxMap.end()){
                        idxMap[pt] = vertices.size();
                        vertices.push_back(pt);
                    }
                }
            }
            for(auto f : faces){
                outFaces.push_back({idxMap[f->a], idxMap[f->b], idxMap[f->c]});
                delete f;
            }
            hull = {vertices,outFaces};
            break;
        }
    }

    return hull;
}
