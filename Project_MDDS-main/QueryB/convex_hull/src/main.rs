// #![allow(unused)]

mod convex_hull;
mod functions;
mod point;
mod plane;
mod edge;
mod hash_stuff;

use convex_hull::*;
use functions::*;
use point::*;

fn main() {
    let points: Vec<Point> = populate_point_vec();

    // let points: Vec<Point> = create_rng_ponts(10_000);

    let mut convex_hull = ConvexHull::new(points.to_vec());

    let start = std::time::Instant::now();
    convex_hull.quick_hull();
    let duration = start.elapsed();

    // println!("{:?}", convex_hull);
    // for plane in convex_hull.planes.iter() {
    //     println!("Plane: ({:?}, {:?}, {:?})", plane.point_a, plane.point_b, plane.point_c);
    // }

    println!("# of planes: {}", convex_hull.planes.len());

    // save the convex hull to a file
    save_to_json("convex_hull.json", &convex_hull);

    println!("Time elapsed in expensive_function() is: {:?}", duration);

}
