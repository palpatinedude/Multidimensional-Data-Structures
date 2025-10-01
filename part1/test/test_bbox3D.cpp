#include "../api/include/bbox3D.h"
#include "../api/include/point3D.h"
#include <cassert>
#include <cmath>
#include <iostream>

void testConstructorsAndValidity() {
    BoundingBox3D invalidBox(0.1f, 0.2f, "", 0.5f, 0.6f, "");

    BoundingBox3D validbox(0.1f, 0.2f, "2023-01-01T00:00:01Z", 0.5f, 0.6f, "2023-01-01T00:00:05Z");

}

void testExpandToIncludePoints() {
    BoundingBox3D box;
    Point3D p1(0.2f, 0.3f, "2023-01-01T00:00:02Z");
    Point3D p2(0.5f, 0.7f, "2023-01-01T00:00:05Z");

    box.expandToInclude(p1);
    box.expandToInclude(std::move(p2));

    assert(std::fabs(box.getMinX() - 0.2f) < 1e-6f);
    assert(std::fabs(box.getMaxX() - 0.5f) < 1e-6f);
    assert(std::fabs(box.getMinY() - 0.3f) < 1e-6f);
    assert(std::fabs(box.getMaxY() - 0.7f) < 1e-6f);
    assert(box.getMinT() == "2023-01-01T00:00:02Z");
    assert(box.getMaxT() == "2023-01-01T00:00:05Z");
}

void testExpandToIncludeBoxes() {
    BoundingBox3D b1(0.0f, 0.0f, "2023-01-01T00:00:00Z", 0.4f, 0.4f, "2023-01-01T00:00:04Z");
    BoundingBox3D b2(0.3f, 0.2f, "2023-01-01T00:00:03Z", 0.7f, 0.6f, "2023-01-01T00:00:06Z");

    b1.expandToInclude(b2);

    assert(std::fabs(b1.getMinX() - 0.0f) < 1e-6f);
    assert(std::fabs(b1.getMaxX() - 0.7f) < 1e-6f);
    assert(std::fabs(b1.getMinY() - 0.0f) < 1e-6f);
    assert(std::fabs(b1.getMaxY() - 0.6f) < 1e-6f);
    assert(b1.getMinT() == "2023-01-01T00:00:00Z");
    assert(b1.getMaxT() == "2023-01-01T00:00:06Z");
}

void testIntersectionAndContainment() {
    BoundingBox3D box1(0.0f, 0.0f, "2023-01-01T00:00:00Z", 0.5f, 0.5f, "2023-01-01T00:00:05Z");
    BoundingBox3D box2(0.4f, 0.4f, "2023-01-01T00:00:02Z", 0.8f, 0.8f, "2023-01-01T00:00:06Z");
    BoundingBox3D box3(0.6f, 0.6f, "2023-01-01T00:00:00Z", 0.9f, 0.9f, "2023-01-01T00:00:04Z");

    assert(box1.intersects(box2));     // Overlapping
    assert(!box1.intersects(box3));    // No overlap

    Point3D p(0.3f, 0.3f, "2023-01-01T00:00:03Z");
    assert(box1.contains(p));          // Inside box1

    Point3D outside(0.6f, 0.6f, "2023-01-01T00:00:03Z");
    assert(!box1.contains(outside));   // Outside box1 spatially
}

void testVolume() {
    BoundingBox3D box(0.0f, 0.0f, "2023-01-01T00:00:00Z", 1.0f, 1.0f, "2023-01-01T00:00:10Z");

    // time difference in seconds = 10
    float expectedVolume = 1.0f * 1.0f * 10.0f;
    assert(std::fabs(box.volume() - expectedVolume) < 1e-5);
}

void testDistance() {
    BoundingBox3D b1(0.0f, 0.0f, "2023-01-01T00:00:00Z", 0.3f, 0.3f, "2023-01-01T00:00:03Z");
    BoundingBox3D b2(1.0f, 1.0f, "2023-01-01T00:00:05Z", 1.2f, 1.2f, "2023-01-01T00:00:08Z");

    float distSq = b1.distanceSquaredTo(b2);
    float dist = b1.distanceTo(b2);

    float expectedDx = 1.0f - 0.3f; // 0.7
    float expectedDy = 1.0f - 0.3f; // 0.7
    float expectedDt = 5 - 3;       // 2 seconds difference
    float expectedDistSq = expectedDx * expectedDx + expectedDy * expectedDy + expectedDt * expectedDt;

    assert(std::fabs(distSq - expectedDistSq) < 1e-5);
    assert(std::fabs(dist - std::sqrt(expectedDistSq)) < 1e-5);
}

void testSpatialDistance() {
    BoundingBox3D b1(0.0f, 0.0f, "2023-01-01T00:00:00Z", 0.3f, 0.3f, "2023-01-01T00:00:03Z");
    BoundingBox3D b2(1.0f, 1.0f, "2023-01-01T00:00:05Z", 1.2f, 1.2f, "2023-01-01T00:00:08Z");

    float dx = 1.0f - 0.3f; // 0.7
    float dy = 1.0f - 0.3f; // 0.7
    float expected = dx * dx + dy * dy;

    assert(std::fabs(b1.spatialDistanceSquared(b2) - expected) < 1e-5);
}

void testPrintAndJSON() {
    BoundingBox3D box(0.1f, 0.2f, "2023-01-01T00:00:01Z", 0.4f, 0.5f, "2023-01-01T00:00:06Z");

    std::cout << "\nTesting print():\n";
    box.print();

    auto j = box.to_json();
    assert(std::fabs(j["minX"].get<float>() - 0.1f) < 1e-6f);
    assert(j["maxT"].get<std::string>() == "2023-01-01T00:00:06Z");
}

int main() {
    testConstructorsAndValidity();
    testExpandToIncludePoints();
    testExpandToIncludeBoxes();
    testIntersectionAndContainment();
    testVolume();
    testDistance();
    testSpatialDistance();
    testPrintAndJSON();

    std::cout << "\nAll BoundingBox3D tests passed successfully.\n";
    return 0;
}
