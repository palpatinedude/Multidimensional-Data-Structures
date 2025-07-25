#include "../include/point3D.h"
#include <iostream>
#include <cmath>

// Default constructor initializes to (0,0,0)
Point3D::Point3D() : x(0.0f), y(0.0f), t(0) {}

// Constructors
Point3D::Point3D(float x, float y, int32_t t) : x(x), y(y), t(t) {}

Point3D::Point3D(const Point3D& other) : x(other.x), y(other.y), t(other.t) {}

Point3D::Point3D(Point3D&& other) noexcept : x(other.x), y(other.y), t(other.t) {
    other.x = 0.0f;
    other.y = 0.0f;
    other.t = 0;
}

// Assignment
Point3D& Point3D::operator=(const Point3D& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        t = other.t;
    }
    return *this;
}

Point3D& Point3D::operator=(Point3D&& other) noexcept {
    if (this != &other) {
        x = other.x;
        y = other.y;
        t = other.t;
        other.x = 0.0f;
        other.y = 0.0f;
        other.t = 0;
    }
    return *this;
}

// Accessors
float Point3D::getX() const { return x; }
float Point3D::getY() const { return y; }
int32_t Point3D::getT() const { return t; }

// Utilities
void Point3D::print() const {
    std::cout << "Point3D(x=" << x << ", y=" << y << ", t=" << t << ")\n";
}

json Point3D::to_json() const {
    return json{{"x", x}, {"y", y}, {"t", t}};
}

void Point3D::validate() const {
    if (x < 0.0f || x > 1.0f)
        std::cerr << "[Warning] Normalized x out of range: " << x << "\n";
    if (y < 0.0f || y > 1.0f)
        std::cerr << "[Warning] Normalized y out of range: " << y << "\n";
    if (t < 0)
        std::cerr << "[Warning] Negative time value: " << t << "\n";
}

// Comparison
bool Point3D::operator==(const Point3D& other) const {
    return x == other.x && y == other.y && t == other.t;
}

// Squared distance calculation (no sqrt, faster for comparisons)
float Point3D::distanceSquaredTo(const Point3D& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    float dt = static_cast<float>(t - other.t);
    return dx * dx + dy * dy + dt * dt;
}

// Euclidean distance calculation
float Point3D::distanceTo(const Point3D& other) const {
    return std::sqrt(distanceSquaredTo(other));
}

