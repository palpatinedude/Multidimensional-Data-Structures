#include "../api/include/point3D.h"
#include <iostream>
#include <cassert>
#include <cmath>

void testConstructors() {
    Point3D p1;
    assert(p1.getX() == 0.0f && p1.getY() == 0.0f && p1.getT() == "");

    Point3D p2(0.5f, 0.8f, "10");
    assert(p2.getX() == 0.5f && p2.getY() == 0.8f && p2.getT() == "10");

    Point3D p3(p2);  // Copy constructor
    assert(p3 == p2);

    Point3D p4(std::move(p2));  // Move constructor
    assert(p4.getX() == 0.5f && p4.getY() == 0.8f && p4.getT() == "10");
}

void testAssignment() {
    Point3D p1(0.1f, 0.2f, "5");
    Point3D p2;
    p2 = p1;  // Copy assignment
    assert(p2 == p1);

    Point3D p3;
    p3 = std::move(p1);  // Move assignment
    assert(p3.getX() == 0.1f && p3.getY() == 0.2f && p3.getT() == "5");
}

void testEquality() {
    Point3D a(0.2f, 0.3f, "3");
    Point3D b(0.2f, 0.3f, "3");
    Point3D c(0.2f, 0.3f, "4");
    assert(a == b);
    assert(!(a == c));
}

void testDistance() {
    Point3D a(0.0f, 0.0f, "0");
    Point3D b(1.0f, 0.0f, "0");
    Point3D c(1.0f, 1.0f, "1");

    float d1 = a.distanceTo(b);
    float d2 = a.distanceTo(c);
    float d3 = a.distanceSquaredTo(c);

    assert(std::fabs(d1 - 1.0f) < 1e-5);
    assert(std::fabs(d2 - std::sqrt(2.0f)) < 1e-5);
    assert(std::fabs(d3 - 2.0f) < 1e-5);
}

void testJSON() {
    Point3D p(0.5f, 0.7f, "20");
    json j = p.to_json();
    assert(j["x"] == 0.5f);
    assert(j["y"] == 0.7f);
    assert(j["t"] == "20");
}

void testValidation() {
    Point3D p1(0.2f, 0.3f, "5");     // Valid
    Point3D p2(100.0f, -0.1f, "");   // Invalid: x out of range, empty t
}

void testPrint() {
    Point3D p(0.25f, 0.75f, "100");
    std::cout << "Testing print(): ";
    p.print();  // Should output: Point3D(x=0.25, y=0.75, t="100")

    json j = p.to_json();
    std::cout << "Testing to_json(): " << j.dump() << std::endl;
}

int main() {
    testConstructors();
    testAssignment();
    testEquality();
    testDistance();
    testJSON();
    testValidation();
    testPrint();

    std::cout << "\nAll Point3D tests passed successfully.\n";
    return 0;
}

