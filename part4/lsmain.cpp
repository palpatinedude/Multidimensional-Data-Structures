#include "ls_intersection.h"
#include "ls_intersection.cpp"
#include <iostream>
#include <vector>

void testBasicIntersections() {
    std::cout << "=== Test 1: Basic Intersections ===\n";
    
    std::vector<lineSeg> segments = {
        lineSeg(point(1, 1), point(4, 4)),  // y = x
        lineSeg(point(1, 4), point(4, 1)),  // y = -x + 5
        lineSeg(point(2, 2), point(5, 5)),  // y = x (parallel to first)
        lineSeg(point(0, 3), point(5, 3))   // y = 3
    };
    
    sweepLine sl(segments);
    auto results = sl.findIntersections();
    
    std::cout << "Expected intersections:\n";
    std::cout << "(2.5, 2.5) - Diagonal lines crossing\n";
    std::cout << "(3.0, 3.0) - First diagonal with horizontal\n"; 
    std::cout << "(2.0, 3.0) - Second diagonal with horizontal\n\n";
    
    std::cout << "Found " << results.size() << " intersections:\n";
    for (const auto& p : results) {
        std::cout << "(" << p.x << ", " << p.y << ")\n";
    }
    std::cout << "------------------------\n";
}

void testParallelSegments() {
    std::cout << "=== Test 2: Parallel Segments ===\n";
    
    std::vector<lineSeg> segments = {
        lineSeg(point(1, 1), point(4, 4)),
        lineSeg(point(2, 2), point(5, 5))  // Parallel, should not intersect
    };
    
    sweepLine sl(segments);
    auto results = sl.findIntersections();
    
    std::cout << "Found " << results.size() << " intersections (expected 0)\n";
    std::cout << "------------------------\n";
}

void testVerticalSegments() {
    std::cout << "=== Test 3: Vertical Segments ===\n";
    
    std::vector<lineSeg> segments = {
        lineSeg(point(2, 1), point(2, 4)),  // Vertical at x=2
        lineSeg(point(1, 2), point(3, 2))   // Horizontal at y=2
    };
    
    sweepLine sl(segments);
    auto results = sl.findIntersections();
    
    std::cout << "Expected intersection: (2.0, 2.0)\n";
    std::cout << "Found " << results.size() << " intersections:\n";
    for (const auto& p : results) {
        std::cout << "(" << p.x << ", " << p.y << ")\n";
    }
    std::cout << "------------------------\n";
}

int main() {
    
    testBasicIntersections();
    testParallelSegments(); 
    testVerticalSegments();
    
    return 0;
}