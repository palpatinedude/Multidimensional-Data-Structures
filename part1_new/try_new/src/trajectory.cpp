#include "../include/trajectory.h"
#include <limits>
#include <cmath>
#include <numeric>
#include <iostream>


Trajectory::Trajectory(std::vector<Point3D> pts, std::string id_)
    : points(std::move(pts)), id(std::move(id_)) {}

Trajectory::Trajectory(std::string id_){
    id = std::move(id_);
}

BoundingBox3D Trajectory::computeBoundingBox() const {
    BoundingBox3D box;
    for (const auto& pt : points)
        box.expandToInclude(pt);
    return box;
}

BoundingBox3D Trajectory::getBoundingBox() const {
    return computeBoundingBox();
}

bool Trajectory::deletePointAt(size_t index) {
    if (index >= points.size()) return false;
    points.erase(points.begin() + index);
    return true;
}

bool Trajectory::updatePointAt(size_t index, const Point3D& newPoint) {
    if (index >= points.size()) return false;
    points[index] = newPoint;
    return true;
}

std::optional<Point3D> Trajectory::getPointAt(size_t index) const {
    if (index >= points.size()) return std::nullopt;
    return points[index];
}

float Trajectory::similarityTo(const Trajectory& other) const {
    if (points.empty() || other.points.empty())
        return std::numeric_limits<float>::max();

    // Case: equal length
    if (points.size() == other.points.size()) {
        float totalDist = 0.0f;
        for (size_t i = 0; i < points.size(); ++i)
            totalDist += points[i].distanceTo(other.points[i]);
        return totalDist / points.size();
    }

    // Fallback: simple DTW
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

float Trajectory::distanceTo(const Trajectory& other) const {
    return similarityTo(other);
}

float Trajectory::length() const {
    if (points.size() < 2) return 0.0f;
    float total = 0.0f;
    for (size_t i = 1; i < points.size(); ++i)
        total += points[i - 1].distanceTo(points[i]);
    return total;
}

int32_t Trajectory::duration() const {
    if (points.size() < 2) return 0;
    return points.back().getT() - points.front().getT();
}

float Trajectory::averageSpeed() const {
    int32_t dur = duration();
    if (dur <= 0) return 0.0f;
    return length() / static_cast<float>(dur);
}

bool Trajectory::isEmpty() const {
    return points.empty();
}

void Trajectory::clear() {
    points.clear();
}

json Trajectory::to_json() const {
    json j;
    j["id"] = id;
    j["points"] = json::array();

    for (const auto& pt : points)
        j["points"].push_back(pt.to_json());

    return j;
}


