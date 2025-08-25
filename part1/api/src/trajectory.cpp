#include "../include/trajectory.h"
#include <limits>
#include <algorithm>
#include <cfloat>

// Utility namespace for timestamp parsing
namespace timeUtil {
    int parseTimestampToSeconds(const std::string& timestamp);
}

// ---------------- Constructors ----------------

// Construct with optional points and identifier
Trajectory::Trajectory(std::vector<Point3D> pts, std::string id_)
    : points(std::move(pts)), id(std::move(id_)), bbox_dirty(true) {
    updateCachedBBox(); // initialize bounding box immediately
}

// Construct with identifier only
Trajectory::Trajectory(std::string id_)
    : id(std::move(id_)), bbox_dirty(true) {}

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
    // Handle empty trajectories
    if (points.empty() || other.points.empty())
        return std::numeric_limits<float>::max();

    // Case 1: Same number of points → mean point-wise distance
    if (points.size() == other.points.size()) {
        float totalDist = 0.0f;
        for (size_t i = 0; i < points.size(); ++i)
            totalDist += points[i].distanceTo(other.points[i]);
        return totalDist / points.size();
    }

    // Case 2: Different sizes → use Dynamic Time Warping (DTW)
    const auto& A = points;
    const auto& B = other.points;
    size_t m = A.size();
    size_t n = B.size();

    // Initialize DP matrix with infinities
    std::vector<std::vector<float>> dtw(m + 1, std::vector<float>(n + 1, std::numeric_limits<float>::max()));
    dtw[0][0] = 0.0f;

    // Fill DP table
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            float cost = A[i - 1].distanceTo(B[j - 1]);
            dtw[i][j] = cost + std::min({dtw[i - 1][j], dtw[i][j - 1], dtw[i - 1][j - 1]});
        }
    }

    // Return normalized distance
    return dtw[m][n] / static_cast<float>(m + n);
}

// Distance is defined as similarity
float Trajectory::distanceTo(const Trajectory& other) const {
    return similarityTo(other);
}

// Define spatio-temporal distance between two trajectories for knn
float Trajectory::spatioTemporalDistanceTo(const Trajectory& other, float timeScale) const {
    if (points.empty() || other.points.empty()) return FLT_MAX;

    float minDist = FLT_MAX; // Start with the maximum possible distance

    // Compare every point in this trajectory with every point in the other trajectory
    for (const auto& p1 : points) {
        for (const auto& p2 : other.points) {
            // Spatial difference (x, y)
            float dx = p1.getX() - p2.getX();
            float dy = p1.getY() - p2.getY();

            // Temporal difference using parseTimestampToSeconds
            float t1 = timeUtil::parseTimestampToSeconds(p1.getT());
            float t2 = timeUtil::parseTimestampToSeconds(p2.getT());
            float dt = (t1 - t2) * timeScale;
            
            float dist = std::sqrt(dx*dx + dy*dy + dt*dt); // Combined spatio-temporal distance using Euclidean formula
            if (dist < minDist) minDist = dist;          // Update minimum distance if current distance is smaller
        }
    }
     std::cout << this->getId() << " vs " << other.getId()
              << " minDist=" << minDist << std::endl;
    // Return the smallest spatio-temporal distance found
    return minDist;
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
int32_t Trajectory::duration() const {
    if (points.size() < 2) return 0;
    return timeUtil::parseTimestampToSeconds(points.back().getT()) -
           timeUtil::parseTimestampToSeconds(points.front().getT());
}

// Compute average speed = length / duration
float Trajectory::averageSpeed() const {
    int32_t dur = duration();
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
