# Trajectory Analysis Project (Part 1 & Part 3)

## Project Overview
This project provides a framework for analyzing the CityTrek-14K trajectory dataset.  
It consists of two independent modules:

1. **Part 1 – 3D R-Trees**: Efficient indexing and querying of spatio-temporal trajectories.  
2. **Part 3 – Convex Hull**: Geometric analysis of trajectories for footprint and coverage.

---

## PART 1 – 3D R-Trees (Trajectory Indexing & Queries)


### Purpose
- Index and query trajectories in 3D space-time `(x = longitude, y = latitude, t = timestamp)`.
- Support range queries, k-nearest neighbors (kNN), and similarity queries.
- Benchmark RTree against naive linear scan for correctness and performance.

### API and Data Structures
- **Point3D**: Single trajectory point `(x, y, t)`.
- **BoundingBox3D**: Axis-aligned 3D bounding box for spatial and temporal pruning.
- **Trajectory**: Contains points, bounding box, optional centroid; provides distance and similarity functions.
- **RTreeNode**: Internal/leaf nodes; leaves store trajectories, internal nodes store bounding boxes.
- **RTree**: Supports bulk-loading, range queries, kNN, and similarity queries in 3D.

### Workflow
1. **Preprocessing (Python)**
   - CSV → compressed Parquet (chunked reading, memory-efficient)
   - Map string IDs to sequential integers
   - Compute bounding boxes per trip `(x_min, x_max, y_min, y_max, t_start, t_end)`

2. **Trajectory Loading (C++)**
   - Read Parquet into `Trajectory` objects
   - Precompute bounding boxes and centroids
   - Maintain a copy for linear scan comparisons

3. **RTree Construction**
   - Bulk-load trajectories (node capacity = 8)
   - Dimensions: x, y = spatial, t = temporal
   - Leaf nodes store trajectory bounding boxes

4. **Queries & Evaluation**
   - **Range Query**: Trajectories intersecting a 3D bounding box
   - **kNN Query**: Top-k nearest trajectories
   - **Similarity Query**: Trajectories similar to a target using approximate pruning
   - Linear scan for benchmarking
   - Metrics saved: execution time, number of trajectories, unique vehicles
   - Results exported to CSV for visualization


### Potential Improvements
- Parallelize preprocessing and bounding box computation
- Support additional attributes or dimensions
- Adaptive thresholds for similarity/kNN
- Incremental RTree updates for new trajectories

---

## PART 3 – Convex Hull (Trajectory Footprint Analysis)

**Purpose**
- Analyze spatial or spatio-temporal coverage of trajectories
- Compute convex hulls for individual trajectories or clusters
- Visualization and potential pruning for trajectory processing

### Implemented Algorithms
- **2D Convex Hull (x, y)**
  - Graham Scan
  - Andrew’s Monotone Chain
  - Divide & Conquer 2D
  - QuickHull 2D
- **3D Convex Hull (x, y, t)**
  - Divide & Conquer 3D
  - QuickHull 3D

### Applications
- Trajectory footprint analysis
- Query result visualization (spatial envelope)
- Spatio-temporal coverage analysis
- Optional candidate pruning before distance computations


---

## Dataset
- **CityTrek-14K**: 14,000 driving trajectories from 280 drivers across Philadelphia, Atlanta, and Memphis
- **Data**: Latitude, longitude, timestamp 
- **Source**: [Kaggle](https://www.kaggle.com/datasets/sobhanmoosavi/citytrek-14k)

---

## Usage Instructions
### Part 1 – RTree
1. Run `preprocess.py` to convert CSV to Parquet
2. Build RTree using `main.cpp`
4. Analyze results via CSV files

### Part 3 – Convex Hull
1. Run `main.cpp`
2. Go to `visualize` folder to run py files to evaluate the
time and space performance of convex hull algorithms




**Note:**  
Part 1 and Part 3 are independent modules. Integration (e.g., convex hulls to optimize RTree queries) is planned for future work.
