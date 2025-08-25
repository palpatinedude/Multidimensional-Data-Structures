#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include "../api/include/trajectory.h"
#include "../api/include/point3D.h"
#include "../api/include/bbox3D.h"

// Small epsilon for floating-point comparisons
static constexpr float EPS = 1e-5f;

void testAddAndGetBoundingBox() {
    std::cout << "\n[TEST] Add points and compute BoundingBox\n";

    Trajectory traj("T1");
    traj.addPoint(Point3D(10.0f, 20.0f, "2023-01-01T10:00:00Z"));
    traj.addPoint(Point3D(15.0f, 25.0f, "2023-01-01T10:05:00Z"));
    traj.addPoint(Point3D(12.0f, 22.0f, "2023-01-01T10:10:00Z"));

    BoundingBox3D bbox = traj.getBoundingBox();
    bbox.print();

    assert(std::fabs(bbox.getMinX() - 10.0f) < EPS);
    assert(std::fabs(bbox.getMaxX() - 15.0f) < EPS);
    assert(std::fabs(bbox.getMinY() - 20.0f) < EPS);
    assert(std::fabs(bbox.getMaxY() - 25.0f) < EPS);
}

void testDeleteAndUpdatePoint() {
    std::cout << "\n[TEST] Delete and update point\n";

    Trajectory traj("T2");
    traj.addPoint(Point3D(1, 1, "2023-01-01T10:00:00Z"));
    traj.addPoint(Point3D(2, 2, "2023-01-01T10:05:00Z"));

    bool deleted = traj.deletePointAt(0);
    assert(deleted);
    auto p0 = traj.getPointAt(0);
    assert(p0.has_value());
    assert(std::fabs(p0->getX() - 2.0f) < EPS);

    bool updated = traj.updatePointAt(0, Point3D(3, 3, "2023-01-01T10:10:00Z"));
    assert(updated);
    auto p0b = traj.getPointAt(0);
    assert(p0b.has_value());
    assert(std::fabs(p0b->getX() - 3.0f) < EPS);
}

void testSimilarityAndDistanceEqualLength() {
    std::cout << "\n[TEST] Similarity and distance between equal-length trajectories\n";

    Trajectory t1("T3");
    t1.addPoint(Point3D(0, 0, "2023-01-01T10:00:00Z"));
    t1.addPoint(Point3D(1, 1, "2023-01-01T10:05:00Z"));

    Trajectory t2("T4");
    t2.addPoint(Point3D(0, 0, "2023-01-01T10:00:00Z"));
    t2.addPoint(Point3D(1, 1, "2023-01-01T10:05:00Z"));

    float sim = t1.similarityTo(t2);
    float dist = t1.distanceTo(t2);

    std::cout << "Similarity: " << sim << "\n";
    std::cout << "Distance: " << dist << "\n";

    assert(std::fabs(sim - dist) < EPS);
    assert(std::fabs(sim - 0.0f) < EPS);
}

void testDTWDifferentLengths() {
    std::cout << "\n[TEST] DTW fallback for different-length trajectories\n";

    // A: 3 points along x-axis: (0,0),(1,0),(2,0)
    // B: 2 points along x-axis: (0,0),(2,0)
    // We computed expected DTW total cost = 1.0, and similarity() divides by (m+n)=5 -> 0.2
    Trajectory A("A");
    A.addPoint(Point3D(0.0f, 0.0f, "2023-01-01T10:00:00Z"));
    A.addPoint(Point3D(1.0f, 0.0f, "2023-01-01T10:01:00Z"));
    A.addPoint(Point3D(2.0f, 0.0f, "2023-01-01T10:02:00Z"));

    Trajectory B("B");
    B.addPoint(Point3D(0.0f, 0.0f, "2023-01-01T10:00:00Z"));
    B.addPoint(Point3D(2.0f, 0.0f, "2023-01-01T10:02:00Z"));

    float sim = A.similarityTo(B);
    std::cout << "DTW-based similarity: " << sim << "\n";

    float expected = 1.0f / 5.0f; // dtw total cost 1.0 divided by (m+n)=5
    assert(std::fabs(sim - expected) < EPS);
}

void testLengthDurationSpeed() {
    std::cout << "\n[TEST] Length, Duration, and Average Speed\n";

    Trajectory traj("T5");
    traj.addPoint(Point3D(0, 0, "2023-01-01T10:00:00Z"));
    traj.addPoint(Point3D(3, 4, "2023-01-01T10:10:00Z")); // distance = 5

    assert(std::fabs(traj.length() - 5.0f) < EPS);
    assert(traj.duration() == 600); // 10 minutes in seconds
    assert(std::fabs(traj.averageSpeed() - (5.0f / 600.0f)) < EPS);
}

void testClearAndEmpty() {
    std::cout << "\n[TEST] Clear and isEmpty\n";

    Trajectory traj("T6");
    traj.addPoint(Point3D(1, 2, "2023-01-01T10:00:00Z"));
    assert(!traj.isEmpty());

    traj.clear();
    assert(traj.isEmpty());

    auto bbox = traj.getBoundingBox();

    // Instead of calling private validate(), check with logical conditions
    // For an invalid bbox, minX > maxX or minY > maxY
    assert(bbox.getMinX() > bbox.getMaxX() || bbox.getMinY() > bbox.getMaxY());
}

void testBoundingBoxInteraction() {
    std::cout << "\n[TEST] Trajectory working with BoundingBox3D directly\n";

    Trajectory traj("T7");
    traj.addPoint(Point3D(1, 1, "2023-01-01T10:00:00Z"));
    traj.addPoint(Point3D(5, 5, "2023-01-01T10:10:00Z"));

    BoundingBox3D box = traj.getBoundingBox();
    Point3D inside(3, 3, "2023-01-01T10:05:00Z");
    Point3D outside(10, 10, "2023-01-01T10:05:00Z");

    assert(box.contains(inside));
    assert(!box.contains(outside));

    BoundingBox3D otherBox(0, 0, "2023-01-01T09:50:00Z", 6, 6, "2023-01-01T10:15:00Z");
    assert(box.intersects(otherBox));
}

void testJsonOutput() {
    std::cout << "\n[TEST] JSON output of trajectory\n";

    Trajectory traj("T8");
    traj.addPoint(Point3D(1, 2, "2023-01-01T10:00:00Z"));
    traj.addPoint(Point3D(3, 4, "2023-01-01T10:05:00Z"));

    json j = traj.to_json();
    std::cout << j.dump(4) << "\n";

    // Basic checks that JSON contains id and points
    assert(j["id"].get<std::string>() == "T8");
    assert(j["points"].is_array());
    assert(j["points"].size() == 2);
}


void testIdAndCachedBBox() {
    std::cout << "\n[TEST] getId, getCachedBBox\n";

    Trajectory traj("ID_TEST");
    assert(traj.getId() == "ID_TEST");

    traj.addPoint(Point3D(0, 0, "2023-01-01T10:00:00Z"));
    auto bbox = traj.getBoundingBox();

    // Instead of private validate(), check indirectly
    assert(bbox.getMinX() <= bbox.getMaxX());
    assert(bbox.getMinY() <= bbox.getMaxY());

    // No setCachedBBox anymore, so just test by adding another point
    traj.addPoint(Point3D(5, 5, "2023-01-01T10:05:00Z"));
    auto updated = traj.getBoundingBox();
    assert(updated.getMaxX() == 5.0f && updated.getMaxY() == 5.0f);
}


void testEqualityOperators() {
    std::cout << "\n[TEST] Equality and inequality operators\n";

    Trajectory t1("EQ1");
    t1.addPoint(Point3D(1,1,"2023-01-01T10:00:00Z"));
    Trajectory t2("EQ1");
    t2.addPoint(Point3D(1,1,"2023-01-01T10:00:00Z"));
    Trajectory t3("EQ2");
    t3.addPoint(Point3D(2,2,"2023-01-01T10:00:00Z"));

    assert(t1 == t2);
    assert(t1 != t3);
}



int main() {
    testAddAndGetBoundingBox();
    testDeleteAndUpdatePoint();
    testSimilarityAndDistanceEqualLength();
    testDTWDifferentLengths();        
    testLengthDurationSpeed();
    testClearAndEmpty();
    testBoundingBoxInteraction();
    testJsonOutput();
    testIdAndCachedBBox();
    testEqualityOperators();

    std::cout << "\n All Trajectory tests passed!\n";
    return 0;
}
