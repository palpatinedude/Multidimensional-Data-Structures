/*
 * trajectory.h
 * -------------
 * Defines the Trajectory class, which represents a sequence of spatiotemporal points (Point3D).
 * Each trajectory has:
 *   - A unique identifier (id)
 *   - A sequence of ordered points (Point3D)
 *   - A lazily-computed bounding box (BoundingBox3D) cached for efficiency
 *
 * Provides:
 *   - Point management (add, delete, update, access)
 *   - Bounding box computation (cached & fresh)
 *   - Trajectory similarity (direct comparison or DTW for uneven sizes)
 *   - Distance, length, duration, and average speed
 *   - Serialization to JSON
 *   - Equality comparison
 *
 * Dependencies:
 *   - point3D.h       (for individual spatiotemporal points)
 *   - bbox3D.h        (for bounding box definition)
 */

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "../include/point3D.h"
#include "../include/bbox3D.h"
#include <vector>
#include <string>
#include <optional>

class Trajectory {
private:
    std::string id;                       // unique identifier of the trajectory
    std::vector<Point3D> points;          // ordered list of spatiotemporal points

    mutable BoundingBox3D cached_bbox;    // cached bounding box for efficiency
    mutable bool bbox_dirty;              // true if cached_bbox needs recomputation

    // Recompute the cached bounding box from points
    void updateCachedBBox() const;

     // Compute centroid (x, y, t) as floats
    void computeCentroid() const;

    // Precomputed centroid
    mutable float centroidX, centroidY, centroidT;


public:
    // ---------------- Constructors ----------------
    Trajectory(std::vector<Point3D> pts = {}, std::string id_ = "");
    explicit Trajectory(std::string id_);

    // ---------------- Bounding Box ----------------
    BoundingBox3D computeBoundingBox() const;  // recompute fresh bounding box
    BoundingBox3D getBoundingBox() const;      // return cached bbox (recomputes if dirty)

    // ---------------- Point Management ----------------
    bool deletePointAt(size_t index);                       // remove point at given index
    bool updatePointAt(size_t index, const Point3D& newPoint); // replace point at given index
    std::optional<Point3D> getPointAt(size_t index) const;  // safely access point
    void addPoint(const Point3D& pt);                       // append a new point
    void reservePoints(size_t n);                           // pre-allocate memory for points

    // ---------------- Similarity / Distance ----------------
    float similarityTo(const Trajectory& other) const;      // mean distance or DTW if different sizes
    float distanceTo(const Trajectory& other) const;        // alias for similarityTo
   // float spatioTemporalDistanceTo(const Trajectory& other, float timeScale = 1e-5f) const; // for knn queries
    float approximateDistance(const Trajectory& other, float timeScale) const;
    float spatioTemporalDistanceTo(const Trajectory& other, float timeScale) const;


    // ---------------- Centroid ----------------
    void precomputeCentroidAndBoundingBox(); // call once after loading
    float getCentroidX() const { return centroidX; }
    float getCentroidY() const { return centroidY; }
    float getCentroidT() const { return centroidT; }


    // ---------------- Utilities ----------------
    float length() const;          // total spatial path length
    float averageSpeed() const;    // average speed (length / duration)
    int64_t duration() const;      // total time duration (last - first timestamp)
    bool isEmpty() const;          // check if trajectory is empty
    void clear();                  // remove all points and reset bbox
  //  float approximateDistance(const Trajectory& other) const;   // Compute a cheap approximate distance to another trajectory

    // ---------------- Comparison ----------------
    bool operator==(const Trajectory& other) const;
    bool operator!=(const Trajectory& other) const;

    // ---------------- Getters ----------------
    const std::string& getId() const;
    const std::vector<Point3D>& getPoints() const;

    // ---------------- Serialization ----------------
    json to_json() const;
};

#endif // TRAJECTORY_H




