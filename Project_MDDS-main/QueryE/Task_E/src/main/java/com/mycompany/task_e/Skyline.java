
package com.mycompany.task_e;


import java.util.ArrayList;
import java.util.List;
import java.util.PriorityQueue;

public class Skyline {
    public static List<Point> findSkylinePoints(List<Point> points) { 
        
        PriorityQueue<Point> pq = new PriorityQueue<>((p1, p2) -> Integer.compare(p2.x, p1.x));
        pq.addAll(points);

        List<Point> skyline = new ArrayList<>();
        Point currentMax = pq.poll(); // Start with the point with the max X
        skyline.add(currentMax);

        while (!pq.isEmpty()) {
            Point next = pq.peek();
            if (currentMax.dominates(next)) {
                // Remove dominated points
                pq.poll();
            } else {
                // Update currentMax and add to skyline
                currentMax = pq.poll();
                skyline.add(currentMax);
            }
        }

        return skyline;
    }
}
