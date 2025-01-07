use crate::functions::*;
use crate::point::Point;
use crate::edge::Edge;
use serde::{Deserialize, Serialize};

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Plane {
    pub point_a: Point,
    pub point_b: Point,
    pub point_c: Point,

    pub normal: Point, // its normal vector is calculated from the three points
 
    pub edge_a: Edge,
    pub edge_b: Edge,
    pub edge_c: Edge,
}

impl PartialEq for Plane {
    fn eq(&self, other: &Self) -> bool {
        let permutations = [
            (self.point_a == other.point_a && self.point_b == other.point_b && self.point_c == other.point_c),
            (self.point_a == other.point_a && self.point_b == other.point_c && self.point_c == other.point_b),
            (self.point_a == other.point_b && self.point_b == other.point_a && self.point_c == other.point_c),
            (self.point_a == other.point_b && self.point_b == other.point_c && self.point_c == other.point_a),
            (self.point_a == other.point_c && self.point_b == other.point_a && self.point_c == other.point_b),
            (self.point_a == other.point_c && self.point_b == other.point_b && self.point_c == other.point_a),
        ];

        permutations.iter().any(|&p| p)
    }
}

impl Plane {
    pub fn new(point_a: Point, point_b: Point, point_c: Point) -> Plane {
        let edge_a = Edge::new(point_a.clone(), point_b.clone());

        let edge_b = Edge::new(point_b.clone(), point_c.clone());

        let edge_c = Edge::new(point_c.clone(), point_a.clone());

        let mut plane = Plane {
            point_a,
            point_b,
            point_c,

            normal: Point::new(None, 0, 0, 0),

            edge_a,
            edge_b,
            edge_c,
        };
        plane.calculate_normal();
        plane
    }

    pub fn calculate_normal(&mut self) {
        let vector1 = subtract_vectors(&self.point_b, &self.point_a);
        let vector2 = subtract_vectors(&self.point_c, &self.point_a);
        self.normal = cross_product(&vector1, &vector2);
    }

    #[allow(dead_code)]
    pub fn get_points(&self) -> Vec<Point> {
        vec![self.point_a.clone(), self.point_b.clone(), self.point_c.clone()]
    }

    pub fn get_edges(&self) -> Vec<Edge> {
        vec![self.edge_a.clone(), self.edge_b.clone(), self.edge_c.clone()]
    }
}