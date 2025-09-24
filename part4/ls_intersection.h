#ifndef LS_INTERSECTION_H
#define LS_INTERSECTION_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <memory>
#include <set>
#include <unordered_set>
#include <queue>


struct point {
    double x, y;
    point(double z = 0, double w = 0) : x(z), y(w) {}
};


struct lineSeg{
    point p1,p2;
    lineSeg(point a, point b) : p1(a), p2(b) {}
    double getY(double x) const ; // Get Y coordinate at given X on the segment

};


enum eventType { START, END, INTERSECTION };

struct event{
    point p; //point where the event occurs
    lineSeg* seg; //which segment is involved
    eventType type; //type of event (START, END, INTERSECTION)
};


struct SegmentComparator{
    double currentX; // Current X position of the sweep line

    bool operator()(const lineSeg* a, const lineSeg* b) const {
        return a->getY(currentX) < b->getY(currentX);
    }
};


class sweepLine
{
private:
    std::priority_queue<event, std::vector<event>, std::greater<event>> eventQueue; // Priority queue of events
    std::set<lineSeg*, SegmentComparator> activeSeg; // Active set of segments intersecting the sweep line
    std::vector<point> intersections; // The intersetions found so far
    std::vector<std::unique_ptr<lineSeg>> segmentStorage; // Owns the segments
    SegmentComparator comparator;

    //functions
    void handleEvent(const event& e);
    void checkNeighbor(std::set<lineSeg*>::iterator it); //Checks intersections between a segment and its immediate neighbors
    point* computeIntersection(lineSeg* s1, lineSeg* s2) const; //helper func for the main algorithm(findIntersections)
    

public:
    sweepLine(const std::vector<lineSeg>& segments);
    ~sweepLine();

    std::vector<point> findIntersections(); //main algorithm function
    void clear();


};

/*sweepLine::sweepLine()
{
}

sweepLine::~sweepLine()
{
}*/



#endif // LS_INTERSECTION_H
