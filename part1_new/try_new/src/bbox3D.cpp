#include "../include/bbox3D.h"
#include <algorithm>
#include <limits>
#include <iostream>

// Default constructor: empty/invalid box
BoundingBox3D::BoundingBox3D()
    : minX(std::numeric_limits<float>::max()),
      minY(std::numeric_limits<float>::max()),
      maxX(std::numeric_limits<float>::lowest()),
      maxY(std::numeric_limits<float>::lowest()),
      minT(std::numeric_limits<int32_t>::max()),
      maxT(std::numeric_limits<int32_t>::min()) {}

BoundingBox3D::BoundingBox3D(float minX, float minY, int32_t minT,
                             float maxX, float maxY, int32_t maxT)
    : minX(minX), minY(minY), maxX(maxX), maxY(maxY), minT(minT), maxT(maxT) {}

bool BoundingBox3D::isValid() const {
    return minX <= maxX && minY <= maxY && minT <= maxT;
}

void BoundingBox3D::expandToInclude(const Point3D& pt) {
    minX = std::min(minX, pt.getX());
    minY = std::min(minY, pt.getY());
    minT = std::min(minT, pt.getT());
    maxX = std::max(maxX, pt.getX());
    maxY = std::max(maxY, pt.getY());
    maxT = std::max(maxT, pt.getT());
}

void BoundingBox3D::expandToInclude(Point3D&& pt) {
    expandToInclude(pt);  // Same logic for now
}

void BoundingBox3D::expandToInclude(const BoundingBox3D& other) {
    minX = std::min(minX, other.minX);
    minY = std::min(minY, other.minY);
    minT = std::min(minT, other.minT);
    maxX = std::max(maxX, other.maxX);
    maxY = std::max(maxY, other.maxY);
    maxT = std::max(maxT, other.maxT);
}

void BoundingBox3D::expandToInclude(BoundingBox3D&& other) {
    expandToInclude(other);  // Same logic for now
}

bool BoundingBox3D::intersects(const BoundingBox3D& other, float epsilon) const {
    return !(maxX + epsilon < other.minX || minX > other.maxX + epsilon ||
             maxY + epsilon < other.minY || minY > other.maxY + epsilon ||
             maxT < other.minT || minT > other.maxT);
}

bool BoundingBox3D::contains(const Point3D& pt, float epsilon) const {
    return (pt.getX() >= minX - epsilon && pt.getX() <= maxX + epsilon &&
            pt.getY() >= minY - epsilon && pt.getY() <= maxY + epsilon &&
            pt.getT() >= minT && pt.getT() <= maxT);
}

float BoundingBox3D::volume() const {
    if (!isValid()) return 0.0f;
    float xSize = maxX - minX;
    float ySize = maxY - minY;
    float tSize = static_cast<float>(maxT - minT);
    return std::max(0.0f, xSize * ySize * tSize);
}

float BoundingBox3D::spatialDistanceSquared(const BoundingBox3D& other) const {
    float dx = std::max(0.0f, std::max(other.minX - maxX, minX - other.maxX));
    float dy = std::max(0.0f, std::max(other.minY - maxY, minY - other.maxY));
    return dx * dx + dy * dy;
}

float BoundingBox3D::distanceSquaredTo(const BoundingBox3D& other) const {
    float dx = std::max(0.0f, std::max(other.minX - maxX, minX - other.maxX));
    float dy = std::max(0.0f, std::max(other.minY - maxY, minY - other.maxY));
    float dt = std::max(0.0f, std::max(static_cast<float>(other.minT - maxT), static_cast<float>(minT - other.maxT)));
    return dx * dx + dy * dy + dt * dt;
}

float BoundingBox3D::distanceTo(const BoundingBox3D& other) const {
    return std::sqrt(distanceSquaredTo(other));
}

// Getters
float BoundingBox3D::getMinX() const { return minX; }
float BoundingBox3D::getMinY() const { return minY; }
float BoundingBox3D::getMaxX() const { return maxX; }
float BoundingBox3D::getMaxY() const { return maxY; }
int32_t BoundingBox3D::getMinT() const { return minT; }
int32_t BoundingBox3D::getMaxT() const { return maxT; }

// JSON output
nlohmann::json BoundingBox3D::to_json() const {
    return {
        {"minX", minX}, {"minY", minY}, {"minT", minT},
        {"maxX", maxX}, {"maxY", maxY}, {"maxT", maxT}
    };
}

void BoundingBox3D::print() const {
    std::cout << "BoundingBox3D(minX=" << minX << ", minY=" << minY << ", minT=" << minT
              << ", maxX=" << maxX << ", maxY=" << maxY << ", maxT=" << maxT << ")\n";
}
