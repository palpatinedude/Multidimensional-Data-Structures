#ifndef POINT3D_H
#define POINT3D_H

#include <cstdint>
#include <iostream>
#include <cmath>
#include "../../json.hpp"

using json = nlohmann::json;

class Point3D {
private:
    float x;  // Normalized longitude
    float y;  // Normalized latitude
    int32_t t; // Time in seconds

public:
    // Constructors
    Point3D();                                // Default constructor (0,0,0)
    Point3D(float x, float y, int32_t t);
    Point3D(const Point3D& other);
    Point3D(Point3D&& other) noexcept;

    // Assignment
    Point3D& operator=(const Point3D& other);
    Point3D& operator=(Point3D&& other) noexcept;

    // Accessors
    float getX() const;
    float getY() const;
    int32_t getT() const;

    // Utility
    void print() const;
    json to_json() const;
    void validate() const;

    // Comparison
    bool operator==(const Point3D& other) const;

    // Distance calculations
    float distanceSquaredTo(const Point3D& other) const;  // squared Euclidean distance (no sqrt)
    float distanceTo(const Point3D& other) const;         // Euclidean distance (calls distanceSquaredTo internally)
};

#endif // POINT3D_H
