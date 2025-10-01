#include "../api/include/point3D.h"
#include <iostream>
#include <cassert>
#include <cmath>


int main() {
    std::cout << "Starting Point3D tests...\n";

    // -------------------- Test Default Constructor --------------------
    Point3D p1;
    assert(p1.getX() == 0.0f);
    assert(p1.getY() == 0.0f);
    assert(p1.getT() == 0);
    p1.print();

    // -------------------- Test Parameterized Constructor --------------------
    int64_t timestamp = 1704067200; // Example: 2024-01-01 00:00:00 UTC
    Point3D p2(12.5f, -45.3f, timestamp);
    assert(p2.getX() == 12.5f);
    assert(p2.getY() == -45.3f);
    assert(p2.getT() == timestamp);
    p2.print();

    // -------------------- Test Copy Constructor --------------------
    Point3D p3(p2);
    assert(p3 == p2);
    p3.print();

    // -------------------- Test Move Constructor --------------------
    Point3D p4(std::move(p3));
    assert(p4.getX() == 12.5f && p4.getY() == -45.3f && p4.getT() == timestamp);
    assert(p3.getX() == 0.0f && p3.getY() == 0.0f && p3.getT() == 0);
    p4.print();
    p3.print();

    // -------------------- Test Copy Assignment --------------------
    Point3D p5;
    p5 = p2;
    assert(p5 == p2);
    p5.print();

    // -------------------- Test Move Assignment --------------------
    Point3D p6;
    p6 = std::move(p5);
    assert(p6 == p2);
    assert(p5.getX() == 0.0f && p5.getY() == 0.0f && p5.getT() == 0);
    p6.print();
    p5.print();

    // -------------------- Test Comparison Operators --------------------
    Point3D p7(12.5f, -45.3f, timestamp);
    assert(p2 == p7);
    assert(p2 != p1);

    // -------------------- Test Distance Calculations --------------------
    Point3D p8(0.0f, 0.0f, 0);
    float dist2 = p2.distanceSquaredTo(p8);
    float dist = p2.distanceTo(p8);
    assert(std::abs(dist2 - (12.5f*12.5f + (-45.3f)*(-45.3f))) < 1e-5);
    assert(std::abs(dist - std::sqrt(12.5f*12.5f + (-45.3f)*(-45.3f))) < 1e-5);
    std::cout << "Distance squared: " << dist2 << "\n";
    std::cout << "Distance: " << dist << "\n";

    // -------------------- Test JSON Conversion --------------------
    json j = p2.to_json();
    assert(j["x"] == 12.5f);
    assert(j["y"] == -45.3f);
    assert(j["t"] == timestamp);
    std::cout << "JSON: " << j.dump() << "\n";

    // -------------------- Edge Case Tests (Validation Warnings) --------------------
    std::cout << "\n--- Edge Case Tests ---\n";

    // Latitude out of range
    Point3D invalidLat(0.0f, 100.0f, timestamp);  // Should trigger warning
    invalidLat.print();

    // Longitude out of range
    Point3D invalidLon(200.0f, 0.0f, timestamp);  // Should trigger warning
    invalidLon.print();

    // Timestamp zero
    Point3D zeroTime(0.0f, 0.0f, 0);              // Should trigger warning
    zeroTime.print();

    // Multiple invalids together
    Point3D allInvalid(300.0f, -120.0f, 0);      // Should trigger multiple warnings
    allInvalid.print();

    std::cout << "\nAll Point3D tests (including edge cases) passed successfully!\n";
    return 0;
}
