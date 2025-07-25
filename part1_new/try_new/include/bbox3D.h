#ifndef BOUNDING_BOX_3D_H
#define BOUNDING_BOX_3D_H

#include "../include/point3D.h"
#include "../../json.hpp"
#include <cstdint>
#include <cmath>

class BoundingBox3D {
private:
    float minX, minY;
    float maxX, maxY;
    int32_t minT, maxT;

public:
    // Constructors
    BoundingBox3D();  // Default constructor (empty box)
    BoundingBox3D(float minX, float minY, int32_t minT,
                  float maxX, float maxY, int32_t maxT);

    // Expansion methods
    void expandToInclude(const Point3D& pt);
    void expandToInclude(Point3D&& pt);
    void expandToInclude(const BoundingBox3D& other);
    void expandToInclude(BoundingBox3D&& other);

    // Query methods
    bool intersects(const BoundingBox3D& other, float epsilon = 1e-6f) const;
    bool contains(const Point3D& pt, float epsilon = 1e-6f) const;
    bool isValid() const;

    // Distance & geometry
    float volume() const;
    float spatialDistanceSquared(const BoundingBox3D& other) const;
    float distanceSquaredTo(const BoundingBox3D& other) const;
    float distanceTo(const BoundingBox3D& other) const;

    // Accessors
    float getMinX() const;
    float getMinY() const;
    float getMaxX() const;
    float getMaxY() const;
    int32_t getMinT() const;
    int32_t getMaxT() const;

    void print() const;
    nlohmann::json to_json() const;
};

#endif // BOUNDING_BOX_3D_H
