#include <bits/stdc++.h>
using namespace std;

// ---------------------- Structs ----------------------
struct Point {
    int x, y;
    bool operator<(const Point& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }
    bool operator==(const Point& other) const {
        return x==other.x && y==other.y;
    }
};

// 3D point
struct Point3 {
    int x, y, z;
    bool operator<(const Point3& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return z < other.z;
    }
    bool operator==(const Point3& other) const {
        return x==other.x && y==other.y && z==other.z;
    }
};

// Global centroid for CCW sorting
pair<int,int> centroid;

// ---------------------- Utility Functions ----------------------

// Determine quadrant for CCW sorting
int quadrant(pair<int,int> p) {
    if(p.first >= 0 && p.second >= 0) return 1;
    if(p.first <= 0 && p.second >= 0) return 2;
    if(p.first <= 0 && p.second <= 0) return 3;
    return 4;
}

// Compare points CCW around centroid
bool ccwCompare(const Point &a, const Point &b) {
    pair<int,int> pa = {a.x - centroid.first, a.y - centroid.second};
    pair<int,int> pb = {b.x - centroid.first, b.y - centroid.second};
    int quadA = quadrant(pa), quadB = quadrant(pb);
    if(quadA != quadB) return quadA < quadB;
    return 1LL*pa.second * pb.first < 1LL*pb.second * pa.first;
}

// Orientation of three 2D points
int orientation(const Point &a, const Point &b, const Point &c) {
    long long val = 1LL*(b.y - a.y)*(c.x - b.x) - 1LL*(b.x - a.x)*(c.y - b.y);
    if(val == 0) return 0;
    return (val > 0) ? 1 : -1;
}

// ---------------------- Brute-force Hull for <=5 points (2D) ----------------------
vector<Point> bruteHull(vector<Point> pts) {
    int n = pts.size();
    set<Point> hullSet;

    for(int i = 0; i < n; ++i) {
        for(int j = i+1; j < n; ++j) {
            int x1 = pts[i].x, y1 = pts[i].y;
            int x2 = pts[j].x, y2 = pts[j].y;
            long long A = (long long)y1 - y2;
            long long B = (long long)x2 - x1;
            long long C = (long long)x1*y2 - (long long)y1*x2;
            int pos=0, neg=0;
            for(int k = 0; k < n; ++k){
                long long val = A*pts[k].x + B*pts[k].y + C;
                if(val >= 0) pos++;
                if(val <= 0) neg++;
            }
            if(pos == n || neg == n){
                hullSet.insert(pts[i]);
                hullSet.insert(pts[j]);
            }
        }
    }

    vector<Point> hull(hullSet.begin(), hullSet.end());

    // Compute centroid
    centroid = {0,0};
    int m = hull.size();
    if (m == 0) return hull;
    for(auto &p : hull) { centroid.first += p.x; centroid.second += p.y; p.x *= m; p.y *= m; }

    // Sort CCW
    sort(hull.begin(), hull.end(), ccwCompare);

    // Rescale
    for(auto &p : hull) { p.x /= m; p.y /= m; }

    return hull;
}

// ---------------------- Merge Two Convex Hulls (2D) ----------------------
vector<Point> mergeHulls(vector<Point> left, vector<Point> right) {
    int nL = left.size(), nR = right.size();
    if(nL == 0) return right;
    if(nR == 0) return left;

    // Find rightmost point of left hull and leftmost of right hull
    int idxL = max_element(left.begin(), left.end(), [](const Point &a, const Point &b){ return a.x < b.x; }) - left.begin();
    int idxR = min_element(right.begin(), right.end(), [](const Point &a, const Point &b){ return a.x < b.x; }) - right.begin();

    // Find upper tangent
    int upL = idxL, upR = idxR;
    bool done = false;
    while(!done) {
        done = true;
        while(orientation(right[upR], left[upL], left[(upL+1)%nL]) >= 0) upL = (upL+1)%nL;
        while(orientation(left[upL], right[upR], right[(upR-1+nR)%nR]) <= 0) { upR = (upR-1+nR)%nR; done=false; }
    }
    int upperL = upL, upperR = upR;

    // Find lower tangent
    upL = idxL; upR = idxR; done = false;
    while(!done){
        done = true;
        while(orientation(left[upL], right[upR], right[(upR+1)%nR]) >= 0) upR = (upR+1)%nR;
        while(orientation(right[upR], left[upL], left[(upL-1+nL)%nL]) <= 0) { upL = (upL-1+nL)%nL; done=false; }
    }
    int lowerL = upL, lowerR = upR;

    // Merge hulls CCW
    vector<Point> merged;
    int ind = upperL; merged.push_back(left[ind]);
    while(ind != lowerL) { ind = (ind+1)%nL; merged.push_back(left[ind]); }
    ind = lowerR; merged.push_back(right[ind]);
    while(ind != upperR) { ind = (ind+1)%nR; merged.push_back(right[ind]); }

    return merged;
}

// ---------------------- Divide & Conquer Convex Hull (2D) ----------------------
vector<Point> divideAndConquer(vector<Point>& points) {
    int n = points.size();
    if(n <= 5) return bruteHull(points);

    vector<Point> left(points.begin(), points.begin()+n/2);
    vector<Point> right(points.begin()+n/2, points.end());

    vector<Point> leftHull = divideAndConquer(left);
    vector<Point> rightHull = divideAndConquer(right);

    return mergeHulls(leftHull, rightHull);
}

// ---------------------- 3D: helpers ----------------------

// Cross product (b - a) x (c - a), returns vector as tuple (long long)
tuple<long long,long long,long long> cross3(const Point3 &a, const Point3 &b, const Point3 &c){
    long long ux = (long long)b.x - a.x;
    long long uy = (long long)b.y - a.y;
    long long uz = (long long)b.z - a.z;
    long long vx = (long long)c.x - a.x;
    long long vy = (long long)c.y - a.y;
    long long vz = (long long)c.z - a.z;
    long long cx = uy * vz - uz * vy;
    long long cy = uz * vx - ux * vz;
    long long cz = ux * vy - uy * vx;
    return {cx, cy, cz};
}

// Dot product n . (p - a)
long long dot_with(const tuple<long long,long long,long long> &n, const Point3 &a, const Point3 &p){
    long long nx, ny, nz; tie(nx, ny, nz) = n;
    long long vx = (long long)p.x - a.x;
    long long vy = (long long)p.y - a.y;
    long long vz = (long long)p.z - a.z;
    return nx*vx + ny*vy + nz*vz;
}

// ---------------------- 3D Brute Hull for <=5 points ----------------------
// Returns vertices of 3D convex hull (set of Point3)
vector<Point3> bruteHull3D(const vector<Point3> &pts){
    int n = pts.size();
    set<Point3> vertices;
    if(n==0) return {};
    if(n<=3) {
        for(auto &p: pts) vertices.insert(p);
        vector<Point3> out(vertices.begin(), vertices.end());
        sort(out.begin(), out.end());
        return out;
    }

    // For every triple (i<j<k), consider plane through i,j,k.
    // If all other points lie strictly on one side (or on plane), then the triangle is a supporting face.
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            for(int k=j+1;k<n;k++){
                // Compute normal n = (pj - pi) x (pk - pi)
                auto nvec = cross3(pts[i], pts[j], pts[k]);
                long long nx, ny, nz; tie(nx,ny,nz) = nvec;
                // Degenerate (collinear) -> skip
                if(nx==0 && ny==0 && nz==0) continue;

                int pos = 0, neg = 0;
                for(int t=0;t<n;t++){
                    long long d = dot_with(nvec, pts[i], pts[t]);
                    if(d > 0) pos++;
                    else if(d < 0) neg++;
                }
                // If all points are on one side (pos==0 or neg==0), triangle is a face
                if(pos == 0 || neg == 0){
                    vertices.insert(pts[i]);
                    vertices.insert(pts[j]);
                    vertices.insert(pts[k]);
                }
            }
        }
    }

    vector<Point3> out(vertices.begin(), vertices.end());
    sort(out.begin(), out.end());
    return out;
}

// ---------------------- Merge Two 3D Hulls ----------------------
// Simple but correct merge: union vertices then recompute hull via brute (works as D&C)
vector<Point3> mergeHulls3D(const vector<Point3> &left, const vector<Point3> &right){
    vector<Point3> combined = left;
    combined.insert(combined.end(), right.begin(), right.end());
    // Remove duplicates
    sort(combined.begin(), combined.end());
    combined.erase(unique(combined.begin(), combined.end(), [](const Point3 &a, const Point3 &b){ return a==b; }), combined.end());
    // Recompute hull on combined set (brute)
    return bruteHull3D(combined);
}

// ---------------------- Divide & Conquer Convex Hull (3D) ----------------------
vector<Point3> divideAndConquer3D(vector<Point3> &points){
    int n = points.size();
    if(n <= 5) {
        // small base case: compute brute hull vertices
        return bruteHull3D(points);
    }

    // Sort by x (stable)
    sort(points.begin(), points.end());
    vector<Point3> left(points.begin(), points.begin()+n/2);
    vector<Point3> right(points.begin()+n/2, points.end());

    vector<Point3> leftHull = divideAndConquer3D(left);
    vector<Point3> rightHull = divideAndConquer3D(right);

    return mergeHulls3D(leftHull, rightHull);
}

// ---------------------- 2D Convex Hull wrapper ----------------------
void convexHull2D(){
    vector<Point> points = {{0,0},{1,-4},{-1,-5},{-5,-3},{-3,-1},{-1,-3},{-2,-2},{-1,-1},{-2,-1},{-1,1}};

    // Remove duplicates
    sort(points.begin(), points.end());
    points.erase(unique(points.begin(), points.end(), [](Point a, Point b){ return a.x==b.x && a.y==b.y; }), points.end());

    vector<Point> hull = divideAndConquer(points);

    cout << "Convex Hull 2D (CCW):\n";
    for(auto &p : hull) cout << p.x << " " << p.y << "\n";

    // Write points to file
    ofstream ptsFile("../results/divideConquer/points2D.txt");
    for(auto &p : points) ptsFile << p.x << " " << p.y << "\n";
    ptsFile.close();

    // Write hull to file
    ofstream hullFile("../results/divideConquer/hull2D.txt");
    for(auto &p : hull) hullFile << p.x << " " << p.y << "\n";
    hullFile.close();
}

// ---------------------- 3D Convex Hull wrapper ----------------------
void convexHull3D(){
    // Example 3D points (you can change to any input)
    vector<Point3> points3 = {
        {0, 0, 0},
        {1, -4, 1},
        {-1, -5, 2},
        {-5, -3, -1},
        {-3, -1, 3},
        {2, 1, -2},
        {3, 2, 1}
    };

    // Remove duplicates & sort
    sort(points3.begin(), points3.end());
    points3.erase(unique(points3.begin(), points3.end(), [](const Point3 &a, const Point3 &b){ return a==b; }), points3.end());

    // Compute 3D hull via divide & conquer
    vector<Point3> hull3 = divideAndConquer3D(points3);

    cout << "\nConvex Hull 3D Vertices:\n";
    for(auto &p : hull3) cout << p.x << " " << p.y << " " << p.z << "\n";

    // Write points to file
    ofstream ptsFile("../results/divideConquer/points3D.txt");
    for(auto &p : points3) ptsFile << p.x << " " << p.y << " " << p.z << "\n";
    ptsFile.close();

    // Save hull vertices to file
    ofstream hull3File("../results/divideConquer/hull3D.txt");
    for(auto &p : hull3) hull3File << p.x << " " << p.y << " " << p.z << "\n";
    hull3File.close();
}

// ---------------------- Main ----------------------
int main(){
    convexHull2D();
    convexHull3D();
    return 0;
}
