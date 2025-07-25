#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "../include/point3D.h"
#include "../include/bbox3D.h"
#include <vector>
#include <string>
#include <optional>

class Trajectory {
public:
    std::string id;
    std::vector<Point3D> points;

    Trajectory(std::vector<Point3D> pts, std::string id_ = "");
    Trajectory(std::string id_);

    // Core API
    BoundingBox3D computeBoundingBox() const;
    BoundingBox3D getBoundingBox() const;  // alias

    bool deletePointAt(size_t index);
    bool updatePointAt(size_t index, const Point3D& newPoint);
    std::optional<Point3D> getPointAt(size_t index) const;

    float similarityTo(const Trajectory& other) const;
    float distanceTo(const Trajectory& other) const;  // alias for similarity

    // Extra utilities
    float length() const;            // total spatial length
    int32_t duration() const;        // time duration
    float averageSpeed() const;

    bool isEmpty() const;
    void clear();

    json to_json() const;
};

#endif // TRAJECTORY_H

