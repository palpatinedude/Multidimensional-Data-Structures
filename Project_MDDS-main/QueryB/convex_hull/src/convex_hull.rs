use serde::{Deserialize, Serialize};
use crate::functions::*;
use crate::point::Point;
use crate::plane::Plane;
use crate::edge::Edge;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ConvexHull {
    pub points: Vec<Point>,
    pub edges: Vec<Edge>,
    pub planes: Vec<Plane>,
}

impl ConvexHull {
    pub fn new(points: Vec<Point>) -> ConvexHull {
        ConvexHull {
            points: points,
            edges: Vec::new(),
            planes: Vec::new(),
        }
    }

    #[allow(dead_code)]
    pub fn iter(&self) -> std::slice::Iter<'_, Point> {
        self.points.iter()
    }
    
    fn init_simplex(&mut self) {
        // get 4 non collinear points
        let mut points: Vec<Point> = Vec::new();
        match find_non_collinear_points(&self.points) {
            Some(p) => points = p,
            None => println!("No non collinear points found"),
        }

        // create the first 4 planes
        let plane1 = Plane::new(points[1].clone(), points[0].clone(), points[2].clone());
        let plane2 = Plane::new(points[0].clone(), points[1].clone(), points[3].clone());
        let plane3 = Plane::new(points[2].clone(), points[0].clone(), points[3].clone());
        let plane4 = Plane::new(points[1].clone(), points[2].clone(), points[3].clone());

        // add the planes to the planes vector
        self.planes.push(plane1);
        self.planes.push(plane2);
        self.planes.push(plane3);
        self.planes.push(plane4);
    }

    pub fn quick_hull(&mut self) {
        self.init_simplex();

        let mut hull: Vec<Plane> = Vec::new();

        let mut i = 0;

        loop {
            if self.planes.is_empty() {
                break;
            } 
            let plane = self.planes.pop().unwrap();
            let mut os: Vec<Point> = Vec::new();
            for point in self.points.iter() {
                if point_above_plane(&plane, &point) {
                    os.push(point.clone());
                }
            }

            // find the farthest point from the plane
            let farthest_point = match farthest_point_from_plane(&plane, &os) {
                Some(p) => p,
                None => {
                    i += 1;
                    hull.push(plane.clone());
                    continue;
                }
            };

            // find the planes that the farthest point is above of
            let mut planes_under_it: Vec<Plane> = Vec::new();
            planes_under_it.push(plane.clone());
            for plane in self.planes.iter() {
                if point_above_plane(&plane, &farthest_point) {
                    planes_under_it.push(plane.clone());
                }
            }

            // get the planes edges
            let mut edges: Vec<Edge> = Vec::new();
            for plane in planes_under_it.iter() {
                edges.append(&mut plane.get_edges());
            }

            // remove the edges that are shared by two planes
            let mut unique_edges: Vec<Edge> = Vec::new();
            for edge in edges.iter() {
                if !unique_edges.contains(edge) {
                    unique_edges.push(edge.clone());
                } else {
                    unique_edges.remove(unique_edges.iter().position(|e| *e == *edge).unwrap());
                }
            }

            // get the points from the edges and create the planes
            let mut planes_to_add: Vec<Plane> = Vec::new();
            for edge in unique_edges.iter() {
                let a = edge.start.clone();
                let b = edge.end.clone();
                let c = farthest_point.clone();
                let plane = Plane::new(a, b, c);
                
                planes_to_add.push(plane);
            }

            // add the planes to the planes vector
            for plane in planes_to_add.iter() {
                self.planes.push(plane.clone());
            }

            for plane in planes_under_it.iter() {
                if self.planes.contains(plane) {
                    self.planes.remove(self.planes.iter().position(|p| *p == *plane).unwrap());
                }
            }

            i += 1;
        }

        println!("Iterations: {}", i);
        self.planes = hull;
        
    }
    
}
