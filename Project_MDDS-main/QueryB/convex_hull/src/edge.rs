use crate::point::Point;
use serde::{Deserialize, Serialize};

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Edge {
    pub start: Point,
    pub end: Point,
}

impl PartialEq for Edge {
    fn eq(&self, other: &Self) -> bool {
        (self.start == other.start && self.end == other.end) || (self.start == other.end && self.end == other.start)
    }
}

impl Edge {
    pub fn new(start: Point, end: Point) -> Edge {
        Edge {
            start: start,
            end: end,
        }
    }
}
