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

double SegmentComparator::currentX = 0.0; //Before processing any events, the sweep line is “sitting” at x = 0


double lineSeg::getY(double x) const {
    if (p1.x == p2.x) { // Vertical line segment ((x1 == x2))
        return p1.y; // or p2.y, they are the same
    }
    //If it is not vertical
    double w = (p2.y - p1.y) / (p2.x - p1.x);
    return p1.y + w * (x - p1.x); //For any x between x₁ and x₂, we can find the corresponding y
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
        if (seg.p2.x < seg.p1.x || (seg.p2.x == seg.p1.x && seg.p2.y < seg.p1.y)) {
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

/*sweepLine::sweepLine(const std::vector<lineSeg>& segments) {

    // Start sweep line at x = 0 (will be updated at each event)
    SegmentComparator::currentX = 0.0;

    // Initialize event queue with segment endpoints
    //int segmentId = 0;
    for (const auto& seg : segments) {
        segmentStorage.push_back(std::make_unique<lineSeg>(seg.p1, seg.p2));
        //segmentId++;
        lineSeg* segPtr = segmentStorage.back().get(); // Get raw pointer to the stored segment

         // Create events using the stored segment's points
        event startEvent{segPtr->p1, segPtr, nullptr, eventType::START};
        event endEvent{segPtr->p2, segPtr, nullptr, eventType::END};


        eventQueue.push({seg.p1, segPtr, nullptr, START});
        eventQueue.push({seg.p2, segPtr,nullptr, END});
    }
}*/

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
    
    // Debug
    std::cout << "        Computing intersection between:" << std::endl;
    std::cout << "          Seg1: (" << x1 << "," << y1 << ") to (" << x2 << "," << y2 << ")" << std::endl;
    std::cout << "          Seg2: (" << x3 << "," << y3 << ") to (" << x4 << "," << y4 << ")" << std::endl;

    //compute the denominator
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    //debug
    std::cout << "          Denominator: " << denom << std::endl;

    if (denom == 0 || fabs(denom) < 1e-10) {
        //debug
        std::cout << "          Lines are parallel/collinear" << std::endl;

        return nullptr; // Parallel lines, no intersection
    }

    double px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
    double py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

    //debug

    std::cout << "          Intersection point: (" << px << ", " << py << ")" << std::endl;

    //check bounding boxes to ensure intersection is within both segments
    //1e-9 is a small tolerance to handle floating-point precision issues
    bool withinSeg1X = (px >= std::min(x1, x2) - 1e-9) && (px <= std::max(x1, x2) + 1e-9);
    bool withinSeg1Y = (py >= std::min(y1, y2) - 1e-9) && (py <= std::max(y1, y2) + 1e-9);
    bool withinSeg2X = (px >= std::min(x3, x4) - 1e-9) && (px <= std::max(x3, x4) + 1e-9);
    bool withinSeg2Y = (py >= std::min(y3, y4) - 1e-9) && (py <= std::max(y3, y4) + 1e-9);
    /*if (px < min(x1, x2) - 1e-9 || px > max(x1, x2) + 1e-9) return nullptr;
    if (px < min(x3, x4) - 1e-9 || px > max(x3, x4) + 1e-9) return nullptr;
    if (py < min(y1, y2) - 1e-9 || py > max(y1, y2) + 1e-9) return nullptr;
    if (py < min(y3, y4) - 1e-9 || py > max(y3, y4) + 1e-9) return nullptr;*/

    //debug

    std::cout << "          Within seg1 bounds: " << (withinSeg1X && withinSeg1Y) << std::endl;
    std::cout << "          Within seg2 bounds: " << (withinSeg2X && withinSeg2Y) << std::endl;

    if (!withinSeg1X || !withinSeg1Y || !withinSeg2X || !withinSeg2Y) {
        std::cout << "          Intersection outside segment bounds" << std::endl;
        return nullptr;
    }

    std::cout << "          Valid intersection found!" << std::endl;

    return new point(px, py); //else return the intersection point which is valid
}


//Check intersections between a segment and its immediate neighbors
void sweepLine::checkNeighbor(std::set<lineSeg*>::iterator it) {
    if (it == activeSeg.end()) return; // No segment to check

    lineSeg* current = *it; // Current segment

    //debug
      std::cout << "    Checking neighbors for segment at Y="
      << current->getY(SegmentComparator::currentX) << std::endl;

    //Check the previous neighbor
    if (it != activeSeg.begin()) {
        auto prevIt = std::prev(it);
        lineSeg* prev = *prevIt;

        //debug
        std::cout << "      Checking with previous neighbor at Y=" 
                  << prev->getY(SegmentComparator::currentX) << std::endl;

     if (point* ip = computeIntersection(current, prev)) {

        //debug
        std::cout << "      Found potential intersection at (" 
                      << ip->x << ", " << ip->y << ")" << std::endl;

        if (ip->x > SegmentComparator::currentX + 1e-9) {
        event interEvent{ *ip, current, prev, INTERSECTION }; 
        eventQueue.push(interEvent);

        //debug
        std::cout << "      Added intersection event!" << std::endl;
            } else {
                std::cout << "      Intersection is in the past, skipping" << std::endl;
            }
    delete ip;
        }

    //debug
    else {
            std::cout << "      No intersection with previous neighbor" << std::endl;
        }

    
}
    
//Check the next neighbor
 auto nextIt = std::next(it);
    if (nextIt != activeSeg.end()) {
        lineSeg* next = *nextIt;

        //debug
        std::cout << "      Checking with next neighbor at Y=" 
                  << next->getY(SegmentComparator::currentX) << std::endl;

        if (point* ip = computeIntersection(current, next)) {

            //debug
            std::cout << "      Found potential intersection at (" 
                      << ip->x << ", " << ip->y << ")" << std::endl;

            if (ip->x > SegmentComparator::currentX + 1e-9) { // only future events
                event interEvent{ *ip, current, next, INTERSECTION };
                eventQueue.push(interEvent);

                //debug
                     std::cout << "      Added intersection event!" << std::endl;
            } else {
                std::cout << "      Intersection is in the past, skipping" << std::endl;
            }
            
            delete ip;
        }

        //debug
           } else {
            std::cout << "      No intersection with next neighbor" << std::endl;
        }
    }

    //debug function to print event queue
    void printEventQueue(std::priority_queue<event, std::vector<event>, eventComparator> pq) {
    std::cout << "Event Queue: size = " << pq.size() << "\n";
    while (!pq.empty()) {
        const auto& e = pq.top();
        std::cout << "(" << e.p.x << ", " << e.p.y << ") Type: " << e.type << "\n";
        pq.pop();
    }
}

//funtion to handle all event types
void sweepLine::handleEvent(const event& e) {

SegmentComparator::currentX = e.p.x; // advance sweep line

   // Debug output
    std::string typeStr = (e.type == START) ? "START" : 
                         (e.type == END) ? "END" : "INTERSECTION";
    std::cout << "Processing " << typeStr << " event at (" 
              << e.p.x << ", " << e.p.y << "), Active segments: " 
              << activeSeg.size() << std::endl;

   switch (e.type){
    
    case START:{
        /*auto it = activeSeg.insert(e.seg1).first; // Insert and get iterator to the new segment
        //debug
        std::cout << "  Inserted segment, now " << activeSeg.size() << " active" << std::endl;
        checkNeighbor(it); // Check for intersections with neighbors
        break;
    }
*/
 auto result = activeSeg.insert(e.seg1);
    auto it = result.first;
    bool inserted = result.second;
    
    if (!inserted) {
        std::cout << "  WARNING: Segment was not inserted (duplicate?)" << std::endl;
    }
    
    std::cout << "  Inserted segment, now " << activeSeg.size() << " active" << std::endl;
    
    // Check intersections with ALL neighbors, not just immediate ones
    
    // Check previous neighbor
    if (it != activeSeg.begin()) {
        auto prevIt = std::prev(it);
        lineSeg* prev = *prevIt;
        lineSeg* current = *it;
        
        std::cout << "    Checking with previous segment" << std::endl;
        if (point* ip = computeIntersection(current, prev)) {
            if (ip->x > SegmentComparator::currentX + 1e-9) {
                event interEvent{ *ip, current, prev, INTERSECTION };
                eventQueue.push(interEvent);
                std::cout << "    Added intersection event at (" << ip->x << ", " << ip->y << ")" << std::endl;
            }
            delete ip;
        }
    }
    
    // Check next neighbor  
    auto nextIt = std::next(it);
    if (nextIt != activeSeg.end()) {
        lineSeg* next = *nextIt;
        lineSeg* current = *it;
        
        std::cout << "    Checking with next segment" << std::endl;
        if (point* ip = computeIntersection(current, next)) {
            if (ip->x > SegmentComparator::currentX + 1e-9) {
                event interEvent{ *ip, current, next, INTERSECTION };
                eventQueue.push(interEvent);
                std::cout << "    Added intersection event at (" << ip->x << ", " << ip->y << ")" << std::endl;
                
                // Debug: Print queue after adding intersection
                printEventQueue(eventQueue);
            }
            delete ip;
        }
    }
    
    // Additional comprehensive check
    for (auto otherIt = activeSeg.begin(); otherIt != activeSeg.end(); ++otherIt) {
        if (otherIt == it) continue; // Skip self
        
        lineSeg* other = *otherIt;
        lineSeg* current = *it;
        
        // Skip if we already checked this pair as neighbors
        if (otherIt == std::prev(it) || otherIt == std::next(it)) continue;
        
        if (point* ip = computeIntersection(current, other)) {
            if (ip->x > SegmentComparator::currentX + 1e-9) {
                event interEvent{ *ip, current, other, INTERSECTION };
                eventQueue.push(interEvent);
                std::cout << "    Added non-adjacent intersection event at (" << ip->x << ", " << ip->y << ")" << std::endl;
            }
            delete ip;
        }
    }
    
    break;
}

    case END:{

    auto it = activeSeg.find(e.seg1);
    if (it != activeSeg.end()) {
        auto prevIt = (it == activeSeg.begin()) ? activeSeg.end() : std::prev(it);
        auto nextIt = std::next(it);

        if (prevIt != activeSeg.end() && nextIt != activeSeg.end()) {
            if (point* ip = computeIntersection(*prevIt, *nextIt)) {
                if (ip->x > SegmentComparator::currentX + 1e-9) {
                    event interEvent{ *ip, *prevIt, *nextIt, INTERSECTION };
                    eventQueue.push(interEvent);
                    //debug
                std::cout << "  Added intersection event at (" << ip->x << ", " << ip->y << ")" << std::endl;
                }
                delete ip;
            }
        }
        activeSeg.erase(it);
        //debug
        std::cout << "  Removed segment, now " << activeSeg.size() << " active" << std::endl;
    }
    break;
    }

    case INTERSECTION:{

      intersections.push_back(e.p); // record intersection

      //debug
    std::cout << "  Found intersection at (" << e.p.x << ", " << e.p.y << ")" << std::endl;

    auto it1 = activeSeg.find(e.seg1);
    auto it2 = activeSeg.find(e.seg2);
    /*if (it1 != activeSeg.end() && it2 != activeSeg.end()) {
       // swap order of the two segments in the active set
        if (SegmentComparator()(*it2, *it1)) std::swap(it1, it2);

        // check new neighbors after swapping
        checkNeighbor(it1);
        checkNeighbor(it2);
    }*/

     if (it1 != activeSeg.end() && it2 != activeSeg.end()) {
        // remove both
        lineSeg* s1 = *it1;
        lineSeg* s2 = *it2;

        //remoe both segments
        activeSeg.erase(it1);
        activeSeg.erase(it2);


 // Temporarily advance sweep line to get correct ordering after intersection
            double originalX = SegmentComparator::currentX;
            SegmentComparator::currentX = e.p.x + 1e-9;

        // reinsert them
        auto newIt1 = activeSeg.insert(s1).first;
        auto newIt2 = activeSeg.insert(s2).first;

        // Restore sweep line position
        SegmentComparator::currentX = originalX;

        std::cout << "    Swapped segments in active set" << std::endl;

        // check neighbors of both
        checkNeighbor(newIt1);
        checkNeighbor(newIt2);
    }

    break;
    }
}
}

// Helper function to remove duplicate intersection points
void removeDuplicateIntersections(std::vector<point>& intersections) {
    if (intersections.empty()) return;

     // Sort points to bring duplicates together
    std::sort(intersections.begin(), intersections.end());
    
    // Use unique to remove consecutive duplicates
    auto last = std::unique(intersections.begin(), intersections.end());
    intersections.erase(last, intersections.end());

}

// Main algorithm function to find all intersections
std::vector<point> sweepLine::findIntersections() {
    intersections.clear();
    
    std::cout << "Starting sweep line algorithm with " << eventQueue.size() << " events\n";
    
    int eventCount = 0;
    while (!eventQueue.empty()) { 
        event currentEvent = eventQueue.top();
        eventQueue.pop();
        
        SegmentComparator::currentX = currentEvent.p.x;
        
       /* // Optional: Print event info for debugging
        if (eventCount % 100 == 0) { // Print every 100 events
            std::cout << "Processing event #" << eventCount 
                      << " at (" << currentEvent.p.x << ", " << currentEvent.p.y << ")"
                      << " Type: " << currentEvent.type 
                      << " Active segments: " << activeSeg.size() << "\n";
        }*/
        
        handleEvent(currentEvent);
        eventCount++;
    }
    
    std::cout << "Algorithm completed. Processed " << eventCount << " events.\n";
    std::cout << "Found " << intersections.size() << " intersection points.\n";
    
    removeDuplicateIntersections(intersections);
    std::cout << "After removing duplicates: " << intersections.size() << " intersections.\n";
    
    return intersections;
}
