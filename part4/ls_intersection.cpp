#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include <queue>
#include <cmath>
#include <iomanip>
#include <tuple>
#include <unordered_set>
#include <memory>

#include "ls_intersection.h"

using namespace std;

double SegmentComparator::currentX = 0.0; //Before processing any events, the sweep line is "sitting" at x = 0

double lineSeg::getY(double x) const {
    if (abs(p1.x - p2.x) < 1e-9) { // Vertical line segment ((x1 == x2))
        return p1.y; // or p2.y, they are the same
    }
    //If it is not vertical
    double slope = (p2.y - p1.y) / (p2.x - p1.x);
    return p1.y + slope * (x - p1.x); //For any x between x₁ and x₂, we can find the corresponding y
}

sweepLine::sweepLine(const std::vector<lineSeg>& segments) {
    // Start sweep line at x = 0 (will be updated at each event)
    SegmentComparator::currentX = 0.0;

    // Initialize event queue with segment endpoints
    for (const auto& seg : segments) {
        // Ensure consistent ordering: leftmost point is p1, rightmost is p2
        point leftPoint = seg.p1;
        point rightPoint = seg.p2;
        
        // If p2 is to the left of p1, swap them
        if (seg.p2.x < seg.p1.x || (abs(seg.p2.x - seg.p1.x) < 1e-9 && seg.p2.y < seg.p1.y)) {
            leftPoint = seg.p2;
            rightPoint = seg.p1;
        }
        
        // Create normalized segment with correct ordering
        segmentStorage.push_back(std::make_unique<lineSeg>(leftPoint, rightPoint));
        lineSeg* segPtr = segmentStorage.back().get();

        // Create events with properly ordered points
        eventQueue.push({leftPoint, segPtr, nullptr, START});
        eventQueue.push({rightPoint, segPtr, nullptr, END});
    }
}

void sweepLine::clear() {
    // clear event queue by swapping with an empty queue
    std::priority_queue<event, std::vector<event>, eventComparator> emptyQueue;
    std::swap(eventQueue, emptyQueue);
    activeSeg.clear();
    intersections.clear();
    segmentStorage.clear();
}

// Destructor
sweepLine::~sweepLine() {
    clear();
}

//Given two segments s1 and s2, compute their intersection point if it exists
point* sweepLine::computeIntersection(lineSeg* s1, lineSeg* s2) const {
    double x1 = s1->p1.x, y1 = s1->p1.y;
    double x2 = s1->p2.x, y2 = s1->p2.y;
    double x3 = s2->p1.x, y3 = s2->p1.y;
    double x4 = s2->p2.x, y4 = s2->p2.y;   

    //compute the denominator
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    
    if (abs(denom) < 1e-10) {
        return nullptr; // Parallel lines, no intersection
    }

    double px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
    double py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

    //check bounding boxes to ensure intersection is within both segments
    //1e-9 is a small tolerance to handle floating-point precision issues
    bool withinSeg1 = (px >= min(x1, x2) - 1e-9) && (px <= max(x1, x2) + 1e-9) &&
                      (py >= min(y1, y2) - 1e-9) && (py <= max(y1, y2) + 1e-9);
    bool withinSeg2 = (px >= min(x3, x4) - 1e-9) && (px <= max(x3, x4) + 1e-9) &&
                      (py >= min(y3, y4) - 1e-9) && (py <= max(y3, y4) + 1e-9);

    if (!withinSeg1 || !withinSeg2) {
        return nullptr;
    }

    return new point(px, py); //else return the intersection point which is valid
}

//Check intersections between a segment and its immediate neighbors
void sweepLine::checkNeighbor(std::set<lineSeg*>::iterator it) {
    if (it == activeSeg.end()) return; // No segment to check

    lineSeg* current = *it; // Current segment

    //Check the previous neighbor
    if (it != activeSeg.begin()) {
        auto prevIt = std::prev(it);
        lineSeg* prev = *prevIt;
        
        if (point* ip = computeIntersection(current, prev)) {
            if (ip->x > SegmentComparator::currentX + 1e-9) { // only future events
                event interEvent{ *ip, current, prev, INTERSECTION }; 
                eventQueue.push(interEvent);
            }
            delete ip;
        }
    }
    
    //Check the next neighbor
    auto nextIt = std::next(it);
    if (nextIt != activeSeg.end()) {
        lineSeg* next = *nextIt;
        
        if (point* ip = computeIntersection(current, next)) {
            if (ip->x > SegmentComparator::currentX + 1e-9) { // only future events
                event interEvent{ *ip, current, next, INTERSECTION };
                eventQueue.push(interEvent);
            }
            delete ip;
        }
    }
}

//function to handle all event types
void sweepLine::handleEvent(const event& e) {
    SegmentComparator::currentX = e.p.x; // advance sweep line

    // Debug output
    std::string typeStr = (e.type == START) ? "START" : 
                         (e.type == END) ? "END" : "INTERSECTION";
    std::cout << "Processing " << typeStr << " event at (" 
              << e.p.x << ", " << e.p.y << ")" << std::endl;

    switch (e.type) {
        case START: {
    auto it = activeSeg.insert(e.seg1).first;
    std::cout << "  Inserted segment, now " << activeSeg.size() << " active" << std::endl;
    
    lineSeg* current = *it;
    
    // Check ALL existing segments, not just neighbors
    for (auto otherIt = activeSeg.begin(); otherIt != activeSeg.end(); ++otherIt) {
        if (otherIt == it) continue; // Skip self
        
        lineSeg* other = *otherIt;
        if (point* ip = computeIntersection(current, other)) {
            if (ip->x > SegmentComparator::currentX + 1e-9 || 
                (abs(ip->x - SegmentComparator::currentX) < 1e-9)) {
                event interEvent{ *ip, current, other, INTERSECTION };
                eventQueue.push(interEvent);
            }
            delete ip;
        }
    }
    break;
}

        case INTERSECTION: {
            intersections.push_back(e.p); // record intersection
            std::cout << "  Found intersection at (" << e.p.x << ", " << e.p.y << ")" << std::endl;

            auto it1 = activeSeg.find(e.seg1);
            auto it2 = activeSeg.find(e.seg2);
            
            if (it1 != activeSeg.end() && it2 != activeSeg.end()) {
                // remove both
                lineSeg* s1 = *it1;
                lineSeg* s2 = *it2;
                
                // Remove both segments
                activeSeg.erase(it1);
                activeSeg.erase(it2);

                // Temporarily advance sweep line to get correct ordering after intersection
                double originalX = SegmentComparator::currentX;
                SegmentComparator::currentX = e.p.x + 1e-9;
                
                // Reinsert segments (now properly ordered)
                auto newIt1 = activeSeg.insert(s1).first;
                auto newIt2 = activeSeg.insert(s2).first;
                
                // Restore sweep line position
                SegmentComparator::currentX = originalX;

                std::cout << "  Swapped segments in active set" << std::endl;

                // check neighbors of both swapped segments
                checkNeighbor(newIt1);
                checkNeighbor(newIt2);
            }
            break;
        }
    }
}

void removeDuplicateIntersections(std::vector<point>& intersections) {
    if (intersections.empty()) return;

    std::sort(intersections.begin(), intersections.end());
    auto last = std::unique(intersections.begin(), intersections.end());
    intersections.erase(last, intersections.end());
}

// Main algorithm function to find all intersections
std::vector<point> sweepLine::findIntersections() {
    intersections.clear();
    
    std::cout << "Starting sweep line algorithm with " << eventQueue.size() << " events" << std::endl;
    
    int eventCount = 0;
    while (!eventQueue.empty()) { 
        event currentEvent = eventQueue.top();
        eventQueue.pop();
        
        SegmentComparator::currentX = currentEvent.p.x;
        
        handleEvent(currentEvent);
        eventCount++;
        
        // Prevent infinite loops
        if (eventCount > 10000) {
            std::cout << "Breaking due to too many events - possible infinite loop" << std::endl;
            break;
        }
    }
    
    std::cout << "Algorithm completed. Processed " << eventCount << " events." << std::endl;
    std::cout << "Found " << intersections.size() << " intersection points." << std::endl;
    
    removeDuplicateIntersections(intersections);
    std::cout << "After removing duplicates: " << intersections.size() << " intersections." << std::endl;
    
    return intersections;
}