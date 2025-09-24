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
    int segmentId = 0;
    for (const auto& seg : segments) {
        segmentStorage.push_back(std::make_unique<lineSeg>(seg.p1, seg.p2, segmentId++));
        lineSeg* segPtr = segmentStorage.back().get(); // Get raw pointer to the stored segment

         // Create events using the stored segment's points
        event startEvent{segPtr->p1, segPtr, eventType::START};
        event endEvent{segPtr->p2, segPtr, eventType::END};


        eventQueue.push({seg.p1, segPtr, START});
        eventQueue.push({seg.p2, segPtr, END});
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


point* sweepLine::computeIntersection(lineSeg* s1, lineSeg* s2) const {}

void sweepLine::checkNeighbor(std::set<lineSeg*>::iterator it) {}


//funtion to handle all event types
void sweepLine::handleEvent(const event& e) {
   switch (e.type){

    case START:{}

    case END:{}

    case INTERSECTION:{}
}
}