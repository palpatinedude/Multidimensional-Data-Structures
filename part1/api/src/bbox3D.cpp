#include "../include/bbox3D.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>

// -------------------- Constructors --------------------

BoundingBox3D::BoundingBox3D()
    : minX(std::numeric_limits<float>::max()),
      minY(std::numeric_limits<float>::max()),
      maxX(std::numeric_limits<float>::lowest()),
      maxY(std::numeric_limits<float>::lowest()),
      minT(0), maxT(0) {}

BoundingBox3D::BoundingBox3D(float minX_, float minY_, int64_t minT_,
                             float maxX_, float maxY_, int64_t maxT_)
    : minX(minX_), minY(minY_), maxX(maxX_), maxY(maxY_), minT(minT_), maxT(maxT_) {
    validate();
}

// -------------------- Validation --------------------

bool BoundingBox3D::validate() const {
    return minX <= maxX && minY <= maxY && minT != 0 && maxT != 0;
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

    if (pt.getT() < minT) minT = pt.getT();
    if (pt.getT() > maxT) maxT = pt.getT();
}

void BoundingBox3D::expandToInclude(Point3D&& pt) { expandToInclude(pt); }

void BoundingBox3D::expandToInclude(const BoundingBox3D& other) {
    if (!other.validate()) return;
    if (!validate()) { *this = other; return; }

    minX = std::min(minX, other.minX);
    minY = std::min(minY, other.minY);
    maxX = std::max(maxX, other.maxX);
    maxY = std::max(maxY, other.maxY);

    if (other.minT < minT) minT = other.minT;
    if (other.maxT > maxT) maxT = other.maxT;
}

void BoundingBox3D::expandToInclude(BoundingBox3D&& other) { expandToInclude(other); }

// -------------------- Geometric queries --------------------

bool BoundingBox3D::intersects(const BoundingBox3D& other, float epsilon) const {
    return !(maxX + epsilon < other.minX || minX > other.maxX + epsilon ||
             maxY + epsilon < other.minY || minY > other.maxY + epsilon ||
             maxT < other.minT || minT > other.maxT);
}

bool BoundingBox3D::contains(const Point3D& pt, float epsilon) const {
    int64_t t = pt.getT();
    return (pt.getX() >= minX - epsilon && pt.getX() <= maxX + epsilon &&
            pt.getY() >= minY - epsilon && pt.getY() <= maxY + epsilon &&
            t >= minT && t <= maxT);
}

float BoundingBox3D::volume() const {
    if (!validate()) return 0.0f;
    float dx = maxX - minX;
    float dy = maxY - minY;
    float dt = static_cast<float>(maxT - minT);
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
        static_cast<float>(other.minT - maxT),
        static_cast<float>(minT - other.maxT)
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
int64_t BoundingBox3D::getMinT() const { return minT; }
int64_t BoundingBox3D::getMaxT() const { return maxT; }

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
