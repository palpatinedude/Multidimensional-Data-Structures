#include "../include/point3D.h"
#include <iostream>
#include <cmath>

// -------------------- Constructors --------------------

// Default constructor: initializes to (0, 0, "")
Point3D::Point3D() : x(0.0f), y(0.0f), t("") {}

// Parameterized constructor: initializes with given values and validates
Point3D::Point3D(float x, float y, const std::string& t) : x(x), y(y), t(t) {
    validate();
}

// Copy constructor: creates a copy of another Point3D
Point3D::Point3D(const Point3D& other) : x(other.x), y(other.y), t(other.t) {}

// Move constructor: transfers ownership and resets source
Point3D::Point3D(Point3D&& other) noexcept : x(other.x), y(other.y), t(std::move(other.t)) {
    other.x = 0.0f;
    other.y = 0.0f;
    other.t.clear();
}

// -------------------- Assignment Operators --------------------

// Copy assignment: assigns values from another Point3D
Point3D& Point3D::operator=(const Point3D& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        t = other.t;
    }
    return *this;
}

// Move assignment: transfers ownership from another Point3D
Point3D& Point3D::operator=(Point3D&& other) noexcept {
    if (this != &other) {
        x = other.x;
        y = other.y;
        t = std::move(other.t);
        other.x = 0.0f;
        other.y = 0.0f;
        other.t.clear();
    }
    return *this;
}

// -------------------- Accessors --------------------
float Point3D::getX() const { return x; }
float Point3D::getY() const { return y; }
std::string Point3D::getT() const { return t; }

// -------------------- Utilities --------------------

// Print the Point3D to the console
void Point3D::print() const {
    std::cout << "Point3D(x=" << x << ", y=" << y << ", t=\"" << t << "\")\n";
}

// Convert the Point3D to JSON format
json Point3D::to_json() const {
    return json{{"x", x}, {"y", y}, {"t", t}};
}

// Validate the coordinates and timestamp
void Point3D::validate() const {
    if (y < -90.0f || y > 90.0f)
        std::cerr << "[Warning] Latitude out of range: " << y << "\n";
    if (x < -180.0f || x > 180.0f)
        std::cerr << "[Warning] Longitude out of range: " << x << "\n";
    if (t.empty())
        std::cerr << "[Warning] Timestamp is empty\n";
}

// -------------------- Comparison Operators --------------------
bool Point3D::operator==(const Point3D& other) const {
    return x == other.x && y == other.y && t == other.t;
}

bool Point3D::operator!=(const Point3D& other) const {
    return !(*this == other);
}


// -------------------- Distance Calculations --------------------

// Euclidean distance to another point
float Point3D::distanceTo(const Point3D& other) const {
    return std::sqrt(distanceSquaredTo(other));
}

// Squared Euclidean distance (more efficient if sqrt not needed)
float Point3D::distanceSquaredTo(const Point3D& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    return dx * dx + dy * dy;
}
