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
    lineSeg* seg1; //which segment is involved
    lineSeg* seg2; //second segment for INTERSECTION events
    eventType type; //type of event (START, END, INTERSECTION)
};

struct eventComparator{ // For priority queue of events
    bool operator()(const event& e1, const event& e2) const {
      if(e1.p.x != e2.p.x)
          return e1.p.x > e2.p.x; //min x first
      if(e1.p.y != e2.p.y)
          return e1.p.y > e2.p.y; //min y first
        return e1.type > e2.type; //START < INTERSECTION < END
    }
};

struct SegmentComparator{
    static double currentX; // Current X position of the sweep line, static so all comparisons use the same sweedpline

    bool operator()(const lineSeg* a, const lineSeg* b) const {
       double y1 = a->getY(currentX);
       double y2 = b->getY(currentX);
       return y1<y2; // Segment with lower Y at currentX comes first
         
    }
};


class sweepLine
{
private:
    std::priority_queue<event, std::vector<event>, eventComparator> eventQueue;// Priority queue of events
    std::set<lineSeg*, SegmentComparator> activeSeg; // Active set of segments intersecting the sweep line
    std::vector<point> intersections; // The intersetions found so far
    std::vector<std::unique_ptr<lineSeg>> segmentStorage; // Owns the segments


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
