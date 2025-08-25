/*
 * bbox3d.h
 * ----------
 * Defines the BBox3D class representing a spatiotemporal bounding box:
 *   - X and Y: spatial bounds (2D)
 *   - T: temporal bounds (time as string)
 *
 * Each bounding box is axis-aligned and provides:
 *   - Constructors, expansion methods
 *   - Geometric queries: intersection, containment, volume, distances
 *   - Accessors, printing, JSON serialization, comparison operators

 */

#ifndef BBOX3D_H
#define BBOX3D_H

#include "../include/point3D.h"
#include <string>
#include <iostream>


class BoundingBox3D {
private:
    // --- Spatial bounds ---
    float minX, minY;
    float maxX, maxY;

    // --- Temporal bounds (timestamps as strings) ---
    std::string minT, maxT;

    // Validate bounding box integrity
    bool validate() const;

public:
    // -------------------- Constructors --------------------
    BoundingBox3D();  // Default: empty bounding box
    BoundingBox3D(float minX, float minY, const std::string& minT,
           float maxX, float maxY, const std::string& maxT);

    // -------------------- Expansion --------------------
    void expandToInclude(const Point3D& pt);
    void expandToInclude(Point3D&& pt);
    void expandToInclude(const BoundingBox3D& other);
    void expandToInclude(BoundingBox3D&& other);

    // -------------------- Geometric queries --------------------
    bool intersects(const BoundingBox3D& other, float epsilon = 1e-6f) const;
    bool contains(const Point3D& pt, float epsilon = 1e-6f) const;

    float volume() const;  // 2D area * temporal duration
    float spatialDistanceSquared(const BoundingBox3D& other) const; // ignores time
    float distanceSquaredTo(const BoundingBox3D& other) const;      // includes time
    float distanceTo(const BoundingBox3D& other) const;             // Euclidean distance

    // -------------------- Accessors --------------------
    float getMinX() const;
    float getMinY() const;
    float getMaxX() const;
    float getMaxY() const;
    std::string getMinT() const;
    std::string getMaxT() const;

    // -------------------- Utilities --------------------
    void print() const;
    json to_json() const;

    // -------------------- Comparison operators --------------------
    bool operator==(const BoundingBox3D& other) const;
    bool operator!=(const BoundingBox3D& other) const;
    friend std::ostream& operator<<(std::ostream& os, const BoundingBox3D& bbox);
};

#endif // BBOX3D_H
