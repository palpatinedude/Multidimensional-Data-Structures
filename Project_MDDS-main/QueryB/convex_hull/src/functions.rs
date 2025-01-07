// #![allow(unused)]
use rand::Rng;
use rand::distributions::Uniform;

use std::io::Write;

use crate::point::*;
use crate::plane::*;
use crate::hash_stuff::*;

const FILE_LOCATION: &str = "../../pol.json";

#[allow(dead_code)]
pub fn create_rng_ponts(it: u32) -> Vec<Point> {
    let vec: Vec<Point> = (0..it)
        .map(|_| Point::new(
            None, 
            rand::thread_rng().sample(Uniform::new(0, 200)),
            rand::thread_rng().sample(Uniform::new(0, 200)),
            rand::thread_rng().sample(Uniform::new(0, 200)),
        ))
        .collect();
    vec
}

pub fn populate_point_vec() -> Vec<Point> {
    // open the FILE_LOCATION json file and read the contents
    let file = std::fs::read_to_string(FILE_LOCATION).unwrap();
    let data: Vec<Data> = serde_json::from_str(&file).unwrap();
    let mut points: Vec<Point> = Vec::new();
    for d in data.iter() {
        points.push(Point::new(
            Some(d.clone()), 
            hash(&d.surname, get_ENV().get_SURNAMES_LENGTH()) as i32,
            d.awards,
            hash(&d.dblp_record, get_ENV().get_DBLP_RECORDS_LENGTH()) as i32, 
        ));
    }
    points
}

pub fn cross_product(a: &Point, b: &Point) -> Point {
    Point::new(
        None,
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    )
}

pub fn dot_product(a: &Point, b: &Point) -> f64 {
    (a.x * b.x + a.y * b.y + a.z * b.z) as f64
}

pub fn magnitude(vector: &Point) -> f64 {
    (vector.x.pow(2) + vector.y.pow(2) + vector.z.pow(2)) as f64
}

pub fn subtract_vectors(a: &Point, b: &Point) -> Point {
    Point::new(
        None,
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    )
}

fn point_to_plane_distance(plane: &Plane, point: &Point) -> f64 {
    let normal = plane.normal.clone();
    let point_vector = subtract_vectors(&point, &plane.point_a);
    let dot = dot_product(&normal, &point_vector);
    let normal_length = magnitude(&normal);
    let distance = dot.abs() / normal_length; // Ensure the distance is non-negative
    distance
}

pub fn farthest_point_from_plane(plane: &Plane, points: &[Point]) -> Option<Point> {
    let mut max_distance = 0.0;
    let mut furthest_point = Point::new(None, 0, 0, 0);
    for point in points.iter() {
        let distance = point_to_plane_distance(&plane, &point);
        if distance > max_distance {
            max_distance = distance;
            furthest_point = point.clone();
        }
    }
    match max_distance {
        #[allow(illegal_floating_point_literal_pattern)]
        0.0 => None,
        _ => Some(furthest_point),
    }
}

pub fn point_above_plane(plane: &Plane, point: &Point) -> bool {
    let vector1 = subtract_vectors(&plane.point_b, &plane.point_a);
    let vector2 = subtract_vectors(&plane.point_c, &plane.point_a);
    let normal = cross_product(&vector1, &vector2);
    let point_vector = subtract_vectors(&point, &plane.point_a);
    dot_product(&normal, &point_vector) > 0.0
}

fn are_collinear(a: &Point, b: &Point, c: &Point) -> bool {
    let vector1 = subtract_vectors(&b, &a);
    let vector2 = subtract_vectors(&c, &a);
    let normal = cross_product(&vector1, &vector2);
    magnitude(&normal) < 0.0001
}

fn are_coplanar(a: &Point, b: &Point, c: &Point, d: &Point) -> bool {
    let vector1 = subtract_vectors(&b, &a);
    let vector2 = subtract_vectors(&c, &a);
    let vector3 = subtract_vectors(&d, &a);
    let normal = cross_product(&vector1, &vector2);
    dot_product(&normal, &vector3) < 0.0001
}

pub fn find_non_collinear_points(points: &Vec<Point>) -> Option<Vec<Point>> {
    let n = points.len();
    if n < 4 {
        panic!("There are less than 4 points in the point vector");
    }
    for i in 0..n {
        for j in i+1..n {
            for k in j+1..n {
                if !are_collinear(&points[i], &points[j], &points[k]) {
                    for l in k+1..n {
                        if !are_coplanar(&points[i], &points[j], &points[k], &points[l]) {
                            return Some(vec![points[i].clone(), points[j].clone(), points[k].clone(), points[l].clone()]);
                        }
                    }
                }
            }
        }
    }
    None
}

pub fn save_to_json<T>(filename: &str, data: &T)
where
    T: serde::Serialize,
{
    let serialized = serde_json::to_string_pretty(data).unwrap();
    let mut file = std::fs::File::create(filename).unwrap();
    file.write_all(serialized.as_bytes()).unwrap();
}

