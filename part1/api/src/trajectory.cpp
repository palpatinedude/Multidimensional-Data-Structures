#include "../include/trajectory.h"
#include <limits>
#include <algorithm>
#include <cfloat>

// ---------------- Constructors ----------------

Trajectory::Trajectory(std::vector<Point3D> pts, std::string id_)
    : points(std::move(pts)), id(std::move(id_)), bbox_dirty(true), centroidX(0), centroidY(0), centroidT(0) {
    precomputeCentroidAndBoundingBox();
}

Trajectory::Trajectory(std::string id_)
    : id(std::move(id_)), bbox_dirty(true), centroidX(0), centroidY(0), centroidT(0) {}

// ---------------- Private Helpers ----------------

// Recompute and update cached bounding box from points
void Trajectory::updateCachedBBox() const {
    cached_bbox = computeBoundingBox();
    bbox_dirty = false;
}

// ---------------- Bounding Box ----------------

// Compute a fresh bounding box from all points
BoundingBox3D Trajectory::computeBoundingBox() const {
    BoundingBox3D box;
    for (const auto& pt : points)
        box.expandToInclude(pt);
    return box;
}

// Return cached bounding box (recomputes if dirty)
BoundingBox3D Trajectory::getBoundingBox() const {
    if (bbox_dirty) {
        updateCachedBBox();
    }
    return cached_bbox;
}

// ---------------- Point Management ----------------

// Remove a point at a given index
bool Trajectory::deletePointAt(size_t index) {
    if (index >= points.size()) return false;
    points.erase(points.begin() + index);
    bbox_dirty = true; // bbox needs updating
    return true;
}

// Update (replace) point at a given index
bool Trajectory::updatePointAt(size_t index, const Point3D& newPoint) {
    if (index >= points.size()) return false;
    points[index] = newPoint;
    bbox_dirty = true;
    return true;
}

// Safely get a point by index (returns std::nullopt if invalid index)
std::optional<Point3D> Trajectory::getPointAt(size_t index) const {
    if (index >= points.size()) return std::nullopt;
    return points[index];
}

// Add a new point to the trajectory
void Trajectory::addPoint(const Point3D& pt) {
    points.push_back(pt);
    bbox_dirty = true;
}

// Reserve memory for points (performance optimization)
void Trajectory::reservePoints(size_t n) {
    points.reserve(n);
}

// ---------------- Similarity / Distance ----------------

// Compute similarity between two trajectories
float Trajectory::similarityTo(const Trajectory& other) const {
    if (points.empty() || other.points.empty())
        return std::numeric_limits<float>::max();

    if (points.size() == other.points.size()) {
        float totalDist = 0.0f;
        for (size_t i = 0; i < points.size(); ++i)
            totalDist += points[i].distanceTo(other.points[i]);
        return totalDist / points.size();
    }

    const auto& A = points;
    const auto& B = other.points;
    size_t m = A.size();
    size_t n = B.size();

    std::vector<std::vector<float>> dtw(m + 1, std::vector<float>(n + 1, std::numeric_limits<float>::max()));
    dtw[0][0] = 0.0f;

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            float cost = A[i - 1].distanceTo(B[j - 1]);
            dtw[i][j] = cost + std::min({dtw[i - 1][j], dtw[i][j - 1], dtw[i - 1][j - 1]});
        }
    }

    return dtw[m][n] / static_cast<float>(m + n);
}

// Distance is defined as similarity
float Trajectory::distanceTo(const Trajectory& other) const {
    return similarityTo(other);
}

// Define spatio-temporal distance between two trajectories for knn
float Trajectory::spatioTemporalDistanceTo(const Trajectory& other, float timeScale) const {
    if (points.empty() || other.points.empty()) return FLT_MAX;

    float minDistSq = std::numeric_limits<float>::infinity();
    const auto& A = points;
    const auto& B = other.points;

    for (const auto& p1 : A) {
        for (const auto& p2 : B) {
            float dx = p1.getX() - p2.getX();
            float dy = p1.getY() - p2.getY();
            float dt = static_cast<float>((p1.getT() - p2.getT()) * static_cast<int64_t>(timeScale));

            float d2 = dx*dx + dy*dy + dt*dt;
            if (d2 < minDistSq) minDistSq = d2;
        }
    }
    return minDistSq;
}

// ---------------- Utilities ----------------

// Compute total path length (sum of segment distances)
float Trajectory::length() const {
    if (points.size() < 2) return 0.0f;
    float total = 0.0f;
    for (size_t i = 1; i < points.size(); ++i)
        total += points[i - 1].distanceTo(points[i]);
    return total;
}

// Compute total time duration of trajectory
int64_t Trajectory::duration() const {
    if (points.size() < 2) return 0;
    return points.back().getT() - points.front().getT();
}

// Compute average speed = length / duration
float Trajectory::averageSpeed() const {
    int64_t dur = duration();
    if (dur <= 0) return 0.0f;
    return length() / static_cast<float>(dur);
}

// Check if trajectory contains no points
bool Trajectory::isEmpty() const {
    return points.empty();
}

// Remove all points and reset cached bounding box
void Trajectory::clear() {
    points.clear();
    cached_bbox = BoundingBox3D(); // reset bbox
    bbox_dirty = true;
}


void Trajectory::computeCentroid() const {
    if (points.empty()) {
        centroidX = centroidY = centroidT = 0;
        return;
    }
    float x = 0, y = 0;
    float t = 0;
    for (auto& pt : points) {
        x += pt.getX();
        y += pt.getY();
        t += static_cast<float>(pt.getT());
    }
    size_t n = points.size();
    centroidX = x / n;
    centroidY = y / n;
    centroidT = t / n;
}


// ---------------- Precompute ----------------

void Trajectory::precomputeCentroidAndBoundingBox() {
    if (points.empty()) {
        cached_bbox = BoundingBox3D();
        bbox_dirty = false;
        centroidX = centroidY = centroidT = 0;
        return;
    }
    updateCachedBBox();
    computeCentroid();
}



float Trajectory::approximateDistance(const Trajectory& other, float timeScale) const {
    float dx = centroidX - other.centroidX;
    float dy = centroidY - other.centroidY;
    float dt = (centroidT - other.centroidT) * timeScale;
    float centroidDistSq = dx*dx + dy*dy + dt*dt;

    float bboxDistSq = getBoundingBox().distanceSquaredTo(other.getBoundingBox());

    return centroidDistSq + bboxDistSq;
}

// ---------------- Serialization ----------------

// Serialize trajectory into JSON
json Trajectory::to_json() const {
    json j;
    j["id"] = id;
    j["points"] = json::array();

    for (const auto& pt : points)
        j["points"].push_back(pt.to_json());

    return j;
}

// ---------------- Getters ----------------

const std::string& Trajectory::getId() const {
    return id;
}

const std::vector<Point3D>& Trajectory::getPoints() const {
    return points;
}

// ---------------- Comparison ----------------

bool Trajectory::operator==(const Trajectory& other) const {
    return id == other.id && points == other.points;
}

bool Trajectory::operator!=(const Trajectory& other) const {
    return !(*this == other);
}


/*
// Compute centroid for the approximate distance
void Trajectory::computeCentroid(float& outX, float& outY, float& outT) const {
    outX = 0.0f; outY = 0.0f; outT = 0.0f;
    if (points.empty()) return;

    for (const auto& pt : points) {
        outX += pt.getX();
        outY += pt.getY();
        outT += static_cast<float>(pt.getT());
    }
    size_t n = points.size();
    outX /= n;
    outY /= n;
    outT /= n;
}*/


/*
// Compute a cheap approximate distance to another trajectory
float Trajectory::approximateDistance(const Trajectory& other, float timeScale) const {
    float x1, y1, t1;
    computeCentroid(x1, y1, t1);

    float x2, y2, t2;
    other.computeCentroid(x2, y2, t2);

    float dx = x1 - x2;
    float dy = y1 - y2;
    float dt = (t1 - t2)*timeScale;
    float centroidDistSq = dx*dx + dy*dy + dt*dt;

    float bboxDistSq = getBoundingBox().distanceSquaredTo(other.getBoundingBox());

    return centroidDistSq + bboxDistSq;
}*/


/*
// Construct with optional points and identifier
Trajectory::Trajectory(std::vector<Point3D> pts, std::string id_)
    : points(std::move(pts)), id(std::move(id_)), bbox_dirty(true) {
    updateCachedBBox(); // initialize bounding box immediately
}

// Construct with identifier only
Trajectory::Trajectory(std::string id_)
    : id(std::move(id_)), bbox_dirty(true) {}*/
