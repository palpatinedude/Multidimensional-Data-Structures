// test_bbox.cpp
#include "../api/include/bbox3D.h"
#include "../api/include/point3D.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    std::cout << "Starting BoundingBox3D tests...\n";

    // -------------------- Test Default Constructor --------------------
    BoundingBox3D bb1;
    bb1.print();
    assert(bb1.getMinT() == 0 && bb1.getMaxT() == 0);

    // -------------------- Test Parameterized Constructor --------------------
    int64_t t1 = 1704067200; // 2024-01-01 00:00:00 UTC
    int64_t t2 = 1704153600; // 2024-01-02 00:00:00 UTC
    BoundingBox3D bb2(10.0f, 20.0f, t1, 15.0f, 25.0f, t2);
    bb2.print();

    // -------------------- Test expandToInclude(Point3D) --------------------
    Point3D p1(12.0f, 22.0f, 1704070000);
    bb2.expandToInclude(p1);
    assert(bb2.contains(p1));
    bb2.print();

    // -------------------- Test expandToInclude(BoundingBox3D) --------------------
    BoundingBox3D bb3(5.0f, 15.0f, 1704050000, 12.0f, 23.0f, 1704080000);
    bb2.expandToInclude(bb3);
    bb2.print();

    // -------------------- Test intersection --------------------
    BoundingBox3D bb4(14.0f, 24.0f, 1704140000, 16.0f, 26.0f, 1704160000);
    assert(bb2.intersects(bb4));
    BoundingBox3D bb5(16.0f, 26.0f, 1704160000, 18.0f, 28.0f, 1704170000);
    assert(!bb2.intersects(bb5));

    // -------------------- Test contains(Point3D) --------------------
    Point3D inside(10.0f, 20.0f, t1);
    Point3D outside(0.0f, 0.0f, 1600000000);
    assert(bb2.contains(inside));
    assert(!bb2.contains(outside));

    // -------------------- Test volume --------------------
    float vol = bb2.volume();
    std::cout << "BoundingBox3D volume: " << vol << "\n";
    assert(vol > 0.0f);

    // -------------------- Test distance calculations --------------------
    float dist2 = bb2.spatialDistanceSquared(bb5);
    float dist3 = bb2.distanceSquaredTo(bb5);
    float dist = bb2.distanceTo(bb5);
    std::cout << "Spatial distance squared: " << dist2 << "\n";
    std::cout << "Distance squared (with time): " << dist3 << "\n";
    std::cout << "Distance (Euclidean): " << dist << "\n";

    // -------------------- Test JSON conversion --------------------
    json j = bb2.to_json();
    std::cout << "BoundingBox3D JSON: " << j.dump() << "\n";
    assert(j["minX"] == bb2.getMinX());

    // -------------------- Test Comparison Operators --------------------
    BoundingBox3D bb6 = bb2;
    assert(bb2 == bb6);
    assert(!(bb2 != bb6));

    // -------------------- Test printing with operator<< --------------------
    std::cout << "BoundingBox3D operator<<: " << bb2 << "\n";

    // -------------------- Edge Case Tests --------------------
    std::cout << "\n--- Edge Case Tests ---\n";

    BoundingBox3D invalidBox1(10.0f, 10.0f, t1, 5.0f, 5.0f, t2);
    invalidBox1.print();

    BoundingBox3D invalidBox2(0.0f, 0.0f, 0, 10.0f, 10.0f, 0);
    invalidBox2.print();

    BoundingBox3D emptyBox;
    Point3D pt(1.0f, 2.0f, 1000);
    emptyBox.expandToInclude(pt);
    emptyBox.print();
    assert(emptyBox.contains(pt));

    BoundingBox3D emptyBox2;
    emptyBox2.expandToInclude(bb2);
    emptyBox2.print();
    assert(emptyBox2 == bb2);

    // -------------------- Dynamic Expansion Test --------------------
    std::cout << "\n--- Dynamic Expansion Test ---\n";
    std::vector<Point3D> points = {
        Point3D(1.0f, 2.0f, 1000),
        Point3D(3.0f, 1.0f, 1200),
        Point3D(-1.0f, 4.0f, 900),
        Point3D(0.0f, 0.0f, 1500)
    };

    BoundingBox3D dynamicBox;
    for (const auto& pt : points) {
        dynamicBox.expandToInclude(pt);
        dynamicBox.print();
    }

    // Check if the final bounding box correctly includes all points
    for (const auto& pt : points) {
        assert(dynamicBox.contains(pt));
    }

     // -------------------- Dynamic Expansion Test with Bounding Boxes --------------------
    std::cout << "\n--- Dynamic Expansion Test with Bounding Boxes ---\n";
    std::vector<BoundingBox3D> boxes = {
        BoundingBox3D(0.0f, 0.0f, 1000, 2.0f, 2.0f, 1100),
        BoundingBox3D(-1.0f, 1.0f, 900, 1.0f, 3.0f, 1200),
        BoundingBox3D(1.5f, -0.5f, 950, 3.0f, 1.5f, 1250)
    };

    BoundingBox3D dynamicBox1;
    for (const auto& bb : boxes) {
        dynamicBox1.expandToInclude(bb);
        dynamicBox1.print();
    }

    // Check that the dynamic bounding box contains all boxes
    for (const auto& bb : boxes) {
        assert(dynamicBox1.getMinX() <= bb.getMinX());
        assert(dynamicBox1.getMinY() <= bb.getMinY());
        assert(dynamicBox1.getMaxX() >= bb.getMaxX());
        assert(dynamicBox1.getMaxY() >= bb.getMaxY());
        assert(dynamicBox1.getMinT() <= bb.getMinT());
        assert(dynamicBox1.getMaxT() >= bb.getMaxT());
    }

    std::cout << "\nAll BoundingBox3D tests (including dynamic expansion with points and boxes) passed successfully!\n";

    return 0;
}
