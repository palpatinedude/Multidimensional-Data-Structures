#include "../include/bbox3D.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>

namespace timeUtil {
    // Convert timestamp string to seconds (placeholder)
    int parseTimestampToSeconds(const std::string& timestamp);
}

// -------------------- Constructors --------------------

BoundingBox3D::BoundingBox3D()
    : minX(std::numeric_limits<float>::max()),
      minY(std::numeric_limits<float>::max()),
      maxX(std::numeric_limits<float>::lowest()),
      maxY(std::numeric_limits<float>::lowest()),
      minT(""), maxT("") {}

BoundingBox3D::BoundingBox3D(float minX_, float minY_, const std::string& minT_,
               float maxX_, float maxY_, const std::string& maxT_)
    : minX(minX_), minY(minY_), maxX(maxX_), maxY(maxY_), minT(minT_), maxT(maxT_) {
    validate();
}

// -------------------- Validation --------------------

bool BoundingBox3D::validate() const {
    /*
    bool valid = true;
    if (minX > maxX) { std::cerr << "[Warning] minX > maxX\n"; valid = false; }
    if (minY > maxY) { std::cerr << "[Warning] minY > maxY\n"; valid = false; }
    if (minT.empty()) { std::cerr << "[Warning] minT is empty\n"; valid = false; }
    if (maxT.empty()) { std::cerr << "[Warning] maxT is empty\n"; valid = false; }
    return valid;*/
    return minX <= maxX && minY <= maxY && !minT.empty() && !maxT.empty();
}

// -------------------- Expansion --------------------

void BoundingBox3D::expandToInclude(const Point3D& pt) {
    if (!validate()) {  // empty box: initialize with point
        minX = maxX = pt.getX();
        minY = maxY = pt.getY();
        minT = maxT = pt.getT();
        return;
    }
    minX = std::min(minX, pt.getX());
    minY = std::min(minY, pt.getY());
    maxX = std::max(maxX, pt.getX());
    maxY = std::max(maxY, pt.getY());

    if (!minT.empty() && timeUtil::parseTimestampToSeconds(pt.getT()) < timeUtil::parseTimestampToSeconds(minT))
        minT = pt.getT();
    if (!maxT.empty() && timeUtil::parseTimestampToSeconds(pt.getT()) > timeUtil::parseTimestampToSeconds(maxT))
        maxT = pt.getT();
}

void BoundingBox3D::expandToInclude(Point3D&& pt) { expandToInclude(pt); }

void BoundingBox3D::expandToInclude(const BoundingBox3D& other) {
    if (!other.validate()) return;
    if (!validate()) { *this = other; return; }

    minX = std::min(minX, other.minX);
    minY = std::min(minY, other.minY);
    maxX = std::max(maxX, other.maxX);
    maxY = std::max(maxY, other.maxY);

    if (timeUtil::parseTimestampToSeconds(other.minT) < timeUtil::parseTimestampToSeconds(minT))
        minT = other.minT;
    if (timeUtil::parseTimestampToSeconds(other.maxT) > timeUtil::parseTimestampToSeconds(maxT))
        maxT = other.maxT;
}

void BoundingBox3D::expandToInclude(BoundingBox3D&& other) { expandToInclude(other); }

// -------------------- Geometric queries --------------------

bool BoundingBox3D::intersects(const BoundingBox3D& other, float epsilon) const {
    return !(maxX + epsilon < other.minX || minX > other.maxX + epsilon ||
             maxY + epsilon < other.minY || minY > other.maxY + epsilon ||
             timeUtil::parseTimestampToSeconds(maxT) < timeUtil::parseTimestampToSeconds(other.minT) ||
             timeUtil::parseTimestampToSeconds(minT) > timeUtil::parseTimestampToSeconds(other.maxT));
}

bool BoundingBox3D::contains(const Point3D& pt, float epsilon) const {
    int tSec = timeUtil::parseTimestampToSeconds(pt.getT());
    return (pt.getX() >= minX - epsilon && pt.getX() <= maxX + epsilon &&
            pt.getY() >= minY - epsilon && pt.getY() <= maxY + epsilon &&
            tSec >= timeUtil::parseTimestampToSeconds(minT) &&
            tSec <= timeUtil::parseTimestampToSeconds(maxT));
}

float BoundingBox3D::volume() const {
    if (!validate()) return 0.0f;
    float dx = maxX - minX;
    float dy = maxY - minY;
    float dt = static_cast<float>(timeUtil::parseTimestampToSeconds(maxT) - timeUtil::parseTimestampToSeconds(minT));
    return std::max(0.0f, dx * dy * dt);
}

float BoundingBox3D::spatialDistanceSquared(const BoundingBox3D& other) const {
    float dx = std::max(0.0f, std::max(other.minX - maxX, minX - other.maxX));
    float dy = std::max(0.0f, std::max(other.minY - maxY, minY - other.maxY));
    return dx * dx + dy * dy;
}

float BoundingBox3D::distanceSquaredTo(const BoundingBox3D& other) const {
    float dx = std::max(0.0f, std::max(other.minX - maxX, minX - other.maxX));
    float dy = std::max(0.0f, std::max(other.minY - maxY, minY - other.maxY));
    float dt = std::max(0.0f, std::max(
        static_cast<float>(timeUtil::parseTimestampToSeconds(other.minT) - timeUtil::parseTimestampToSeconds(maxT)),
        static_cast<float>(timeUtil::parseTimestampToSeconds(minT) - timeUtil::parseTimestampToSeconds(other.maxT))
    ));
    return dx*dx + dy*dy + dt*dt;
}

float BoundingBox3D::distanceTo(const BoundingBox3D& other) const {
    return std::sqrt(distanceSquaredTo(other));
}

// -------------------- Accessors --------------------

float BoundingBox3D::getMinX() const { return minX; }
float BoundingBox3D::getMinY() const { return minY; }
float BoundingBox3D::getMaxX() const { return maxX; }
float BoundingBox3D::getMaxY() const { return maxY; }
std::string BoundingBox3D::getMinT() const { return minT; }
std::string BoundingBox3D::getMaxT() const { return maxT; }

// -------------------- Utilities --------------------

json BoundingBox3D::to_json() const {
    return {{"minX", minX}, {"minY", minY}, {"minT", minT},
            {"maxX", maxX}, {"maxY", maxY}, {"maxT", maxT}};
}

void BoundingBox3D::print() const {
    std::cout << "BoundingBox3D(minX=" << minX << ", minY=" << minY << ", minT=" << minT
              << ", maxX=" << maxX << ", maxY=" << maxY << ", maxT=" << maxT << ")\n";
}

// -------------------- Comparison --------------------

bool BoundingBox3D::operator==(const BoundingBox3D& other) const {
    return minX == other.minX && minY == other.minY &&
           maxX == other.maxX && maxY == other.maxY &&
           minT == other.minT && maxT == other.maxT;
}

bool BoundingBox3D::operator!=(const BoundingBox3D& other) const { return !(*this == other); }

std::ostream& operator<<(std::ostream& os, const BoundingBox3D& box) {
    os << "BoundingBox3D(minX=" << box.minX << ", minY=" << box.minY << ", minT=" << box.minT
       << ", maxX=" << box.maxX << ", maxY=" << box.maxY << ", maxT=" << box.maxT << ")";
    return os;
}
