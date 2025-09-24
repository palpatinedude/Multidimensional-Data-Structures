/*
 * quickHull3D.cpp
 * -------------------
 * Implements the QuickHull algorithm for computing a convex hull in 3D.
 *
 * Key idea of QuickHull in 3D:
 * 1. Find an initial tetrahedron (4 non-coplanar points).
 * 2. Assign all other points to the "outside sets" of faces (points lying outside a face).
 * 3. While there exists a face with outside points:
 *      - Pick the farthest point from that face.
 *      - Identify all faces visible from this point.
 *      - Find the "horizon" edges (boundary between visible and non-visible faces).
 *      - Remove visible faces, and create new faces connecting horizon edges to the farthest point.
 *      - Redistribute outside points to the new faces.
 * 4. Return the remaining valid faces of the hull.
 * -------------------
 * - Faces are represented as oriented triangles with outward normals.
 * - Final result: list of hull vertices and triangular faces.
 */

#include "../../common.h"
#include "../../allAlgorithms.h"
#include <unordered_set>
#include <memory>
#include <array>
#include <algorithm>
#include <queue>
using namespace std;

double EPS3D = 1e-12;

// ---------------------- Face Structure ----------------------
struct Face3D {
    int a, b, c;  // vertex indices (into input points array)
    tuple<double,double,double> normal; // normalized face normal
    double offset;                      // plane equation offset
    unordered_set<int> outside;         // set of points outside this face
    bool valid;                         // if face is still part of hull

    Face3D(int A, int B, int C) : a(A), b(B), c(C), offset(0), valid(true) {}

    // Compute face plane (normal + offset) given the 3 vertices
    void compute_plane(const vector<Point3>& pts) {
        auto nvec = cross3(pts[a], pts[b], pts[c]); // normal vector
        double nx, ny, nz;
        tie(nx, ny, nz) = nvec;
        double len = sqrt(nx*nx + ny*ny + nz*nz);
        if(len < EPS3D) { normal = {0,0,0}; offset = 0; return; }
        normal = {nx/len, ny/len, nz/len};
        offset = nx/len*pts[a].x + ny/len*pts[a].y + nz/len*pts[a].z;
    }

    // Signed distance of point from plane (positive = in front of face)
    double distance(const Point3& p) const {
        double nx, ny, nz;
        tie(nx, ny, nz) = normal;
        return nx*p.x + ny*p.y + nz*p.z - offset;
    }
};

// ---------------------- QuickHull3D Implementation ----------------------
class QuickHull3D {
    const vector<Point3>& pts;                   // input points
    vector<shared_ptr<Face3D>> faces;            // current hull faces

    // Create and register a face
    shared_ptr<Face3D> make_face(int a, int b, int c) {
        auto f = make_shared<Face3D>(a, b, c);
        f->compute_plane(pts);
        faces.push_back(f);
        return f;
    }

    // Remove invalid faces from list
    void cleanup_faces() {
        vector<shared_ptr<Face3D>> nf;
        nf.reserve(faces.size());
        for(auto &f: faces)
            if(f->valid) nf.push_back(f);
        faces.swap(nf);
    }

public:
    QuickHull3D(const vector<Point3>& P): pts(P) {}

    // Build convex hull faces (as index triplets)
    vector<array<int,3>> build() {
        int N = pts.size();
        vector<array<int,3>> hullFaces;
        if(N < 4) return hullFaces; // need at least 4 points

        // ---------------------- 1. Find initial tetrahedron ----------------------
        int min_x = 0, max_x = 0;
        for(int i=1;i<N;i++){
            if(pts[i].x < pts[min_x].x) min_x = i;
            if(pts[i].x > pts[max_x].x) max_x = i;
        }
        if(min_x == max_x) return hullFaces; // all points identical in X

        // Pick 3rd point farthest from the line (min_x, max_x)
        double bestd = -1; int third = -1;
        double dx = pts[max_x].x - pts[min_x].x;
        double dy = pts[max_x].y - pts[min_x].y;
        double dz = pts[max_x].z - pts[min_x].z;
        for(int i=0;i<N;i++){
            double cx = (pts[i].y-pts[min_x].y)*dz - (pts[i].z-pts[min_x].z)*dy;
            double cy = (pts[i].z-pts[min_x].z)*dx - (pts[i].x-pts[min_x].x)*dz;
            double cz = (pts[i].x-pts[min_x].x)*dy - (pts[i].y-pts[min_x].y)*dx;
            double d = sqrt(cx*cx + cy*cy + cz*cz);
            if(d > bestd){ bestd = d; third = i; }
        }
        if(third == -1) return hullFaces;

        // Pick 4th point farthest from plane (min_x, max_x, third)
        auto nvec = cross3(pts[min_x], pts[max_x], pts[third]);
        double nx, ny, nz; tie(nx, ny, nz) = nvec;
        double len = sqrt(nx*nx + ny*ny + nz*nz);
        if(len < EPS3D) return hullFaces;
        nx/=len; ny/=len; nz/=len;
        bestd = -1; int fourth = -1;
        for(int i=0;i<N;i++){
            double d = fabs((pts[i].x-pts[min_x].x)*nx +
                            (pts[i].y-pts[min_x].y)*ny +
                            (pts[i].z-pts[min_x].z)*nz);
            if(d > bestd){ bestd = d; fourth = i; }
        }
        if(fourth == -1) return hullFaces;

        // Build initial tetrahedron faces
        array<array<int,3>,4> combos = {{
            {min_x, max_x, third},
            {min_x, fourth, max_x},
            {min_x, third, fourth},
            {max_x, fourth, third}
        }};
        for(auto &t: combos) make_face(t[0], t[1], t[2]);

        // ---------------------- 2. Assign outside points to faces ----------------------
        unordered_set<int> unassigned;
        for(int i=0;i<N;i++) unassigned.insert(i);
        unassigned.erase(min_x); unassigned.erase(max_x);
        unassigned.erase(third); unassigned.erase(fourth);

        for(int p: vector<int>(unassigned.begin(), unassigned.end())){
            double maxd = EPS3D; shared_ptr<Face3D> best = nullptr;
            for(auto &f: faces){
                double d = f->distance(pts[p]);
                if(d > maxd){ maxd = d; best = f; }
            }
            if(best) { best->outside.insert(p); unassigned.erase(p); }
        }

        // ---------------------- 3. Expand hull iteratively ----------------------
        while(true){
            // Pick face with outside points
            shared_ptr<Face3D> face = nullptr;
            for(auto &f: faces){ if(f->valid && !f->outside.empty()){ face = f; break; } }
            if(!face) break;

            // Pick farthest outside point from this face
            int farthest = -1; double maxd = -1;
            for(int p: face->outside){
                double d = face->distance(pts[p]);
                if(d > maxd){ maxd = d; farthest = p; }
            }
            if(farthest == -1){ face->outside.clear(); continue; }

            // 3a. Identify visible faces from farthest point
            vector<shared_ptr<Face3D>> visible;
            for(auto &f: faces)
                if(f->valid && f->distance(pts[farthest]) > EPS3D)
                    visible.push_back(f);

            // 3b. Find horizon edges (edges shared by exactly one visible face)
            unordered_map<long long,int> edgeCount;
            auto edgeKey=[](int u,int v){ int a=min(u,v),b=max(u,v); return ((long long)a<<32)|b; };
            for(auto &vf: visible){
                array<int,3> es={vf->a,vf->b,vf->c};
                for(int i=0;i<3;i++){
                    int u=es[i], v=es[(i+1)%3];
                    edgeCount[edgeKey(u,v)]++;
                }
            }

            vector<pair<int,int>> horizon;
            for(auto &vf: visible){
                array<int,3> es={vf->a,vf->b,vf->c};
                for(int i=0;i<3;i++){
                    int u=es[i], v=es[(i+1)%3];
                    if(edgeCount[edgeKey(u,v)] == 1) horizon.emplace_back(u,v);
                }
            }

            // 3c. Remove visible faces and collect their outside points
            unordered_set<int> affectedPoints;
            for(auto &vf: visible){
                vf->valid = false;
                for(int p: vf->outside) affectedPoints.insert(p);
                vf->outside.clear();
            }

            // 3d. Create new faces from horizon edges and farthest point
            vector<shared_ptr<Face3D>> newFaces;
            for(auto &e: horizon){
                auto nf = make_face(e.first, e.second, farthest);
                newFaces.push_back(nf);
            }

            // 3e. Reassign affected points to new faces
            affectedPoints.erase(farthest);
            for(int p: affectedPoints){
                double maxd = EPS3D; shared_ptr<Face3D> best = nullptr;
                for(auto &nf: newFaces){
                    double d = nf->distance(pts[p]);
                    if(d > maxd){ maxd = d; best = nf; }
                }
                if(best) best->outside.insert(p);
            }

            cleanup_faces();
        }

        // ---------------------- 4. Collect final hull faces ----------------------
        for(auto &f: faces){
            if(f->valid) hullFaces.push_back({f->a,f->b,f->c});
        }

        return hullFaces;
    }
};

// ---------------------- QuickHull main function ----------------------
Hull3D quickHull3D(vector<Point3>& points){
    QuickHull3D qh(points);
    vector<array<int,3>> faces = qh.build();

    // Collect unique vertices used in hull
    unordered_map<int,int> oldToNew;
    vector<Point3> verts;
    int idx = 0;
    for(auto &f : faces){
        for(int i=0;i<3;i++){
            if(!oldToNew.count(f[i])){
                oldToNew[f[i]] = idx++;
                verts.push_back(points[f[i]]);
            }
        }
    }

    // Reindex faces to match new vertex list
    vector<array<int,3>> newFaces;
    for(auto &f : faces){
        newFaces.push_back({oldToNew[f[0]], oldToNew[f[1]], oldToNew[f[2]]});
    }

    return {verts, newFaces};
}
