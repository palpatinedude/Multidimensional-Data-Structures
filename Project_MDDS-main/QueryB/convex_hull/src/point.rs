use crate::hash_stuff::Data;
use serde::{Deserialize, Serialize};

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Point {
    pub data: Option<Data>,
    pub x: i32,
    pub y: i32,
    pub z: i32,
}

impl Point {
    pub fn new(data: Option<Data>, x: i32, y: i32, z: i32) -> Point {
        Point {
            data: data,
            x: x,
            y: y,
            z: z,
        }
    }
}

impl PartialEq for Point {
    fn eq(&self, other: &Self) -> bool {
        self.x == other.x && self.y == other.y && self.z == other.z
    }
}
