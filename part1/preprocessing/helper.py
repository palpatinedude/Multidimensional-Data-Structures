"""
query_trajectories_range.py

Query trajectories by a timestamp range from preprocessed CityTrek-14K data.
Outputs:
- Number of trajectories in the range
- Number of unique vehicles in the range
- Trajectory IDs
"""
"""
import pandas as pd
import dask.dataframe as dd

# -----------------------------
# Configuration
# -----------------------------
PARQUET_DIR = "trajectory_data_parquet"  # Folder with Parquet chunks
# Timestamp range to query
START_TIMESTAMP = "2017-07-20T10:15:00"
END_TIMESTAMP = "2017-09-05T18:45:00"

# -----------------------------
# Load Parquet data
# -----------------------------
print("Loading preprocessed trajectories from Parquet...")
ddf = dd.read_parquet(PARQUET_DIR, engine="pyarrow")
ddf["t"] = dd.to_datetime(ddf["t"], errors="coerce")  # Ensure datetime type
ddf = ddf.dropna(subset=["t"])

# -----------------------------
# Filter by timestamp range
# -----------------------------
start_time = pd.to_datetime(START_TIMESTAMP)
end_time = pd.to_datetime(END_TIMESTAMP)
filtered = ddf[ddf["t"].between(start_time, end_time)]

# Compute results
filtered_df = filtered.compute()
trajectory_ids = filtered_df.apply(lambda row: f"{row['vehicle_id']}_{row['trip_id']}", axis=1).unique()
unique_vehicles = filtered_df["vehicle_id"].nunique()

# -----------------------------
# Print results
# -----------------------------
print(f"Query time range: {START_TIMESTAMP} --> {END_TIMESTAMP}")
print(f"Number of trajectories in this range: {len(trajectory_ids)}")
print(f"Number of unique vehicles in this range: {unique_vehicles}")
print("Trajectory IDs:")
for traj_id in trajectory_ids:
    print(traj_id)

"""

"""
helper.py

Spatio-temporal kNN on CityTrek-14K dataset with unique-vehicle constraint.
Uses all points to ensure N_NEIGHBORS unique vehicles.
"""

import pandas as pd
import dask.dataframe as dd
from sklearn.neighbors import NearestNeighbors
import numpy as np

# -----------------------------
# Configuration
# -----------------------------
PARQUET_DIR = "trajectory_data_parquet"
N_NEIGHBORS = 5          # Number of unique-vehicle neighbors to return
TIME_SCALE = 1e-5        # Scale factor for timestamp
N_LOOKUP = 6000          # Number of kNN candidates to consider

# -----------------------------
# Load Data
# -----------------------------
print("Loading preprocessed trajectories...")
ddf = dd.read_parquet(PARQUET_DIR, engine="pyarrow")
ddf["t"] = dd.to_datetime(ddf["t"], errors="coerce")
df = ddf.dropna(subset=["x", "y", "t"]).compute()

print(f"Total points available: {len(df)}")
print(f"Unique vehicles: {df['vehicle_id'].nunique()}")

# Convert timestamps to numeric seconds
df["t_numeric"] = df["t"].astype("int64") // 1e9

# Prepare features: [x, y, scaled_time]
X = np.vstack([
    df["x"].values,
    df["y"].values,
    df["t_numeric"].values * TIME_SCALE
]).T

# Adjust N_LOOKUP if larger than total points
N_LOOKUP = min(N_LOOKUP, len(X))

# -----------------------------
# Fit kNN model
# -----------------------------
knn = NearestNeighbors(n_neighbors=N_LOOKUP, algorithm="auto")
knn.fit(X)

# -----------------------------
# Query function
# -----------------------------
def query_knn(query_idx=0):
    query_point = X[query_idx].reshape(1, -1)
    query_vehicle = df.iloc[query_idx]["vehicle_id"]
    query_trajectory_id = f"{query_vehicle}_{df.iloc[query_idx]['trip_id']}"

    distances, indices = knn.kneighbors(query_point, n_neighbors=N_LOOKUP)

    unique_neighbors = []
    seen_vehicles = set()
    for dist, idx in zip(distances[0], indices[0]):
        neighbor = df.iloc[idx]
        vehicle_id = neighbor["vehicle_id"]
        trajectory_id = f"{vehicle_id}_{neighbor['trip_id']}"

        # Skip same vehicle or duplicates
        if vehicle_id == query_vehicle or vehicle_id in seen_vehicles:
            continue

        unique_neighbors.append((trajectory_id, neighbor["x"], neighbor["y"], neighbor["t"], dist))
        seen_vehicles.add(vehicle_id)

        if len(unique_neighbors) >= N_NEIGHBORS:
            break

    # Print results
    print("\nQuery Point:")
    print(f"trajectory_id={query_trajectory_id}, lon={df.iloc[query_idx]['x']:.4f}, "
          f"lat={df.iloc[query_idx]['y']:.4f}, t={df.iloc[query_idx]['t']}")

    print(f"\nNearest {N_NEIGHBORS} neighbors (unique vehicles):")
    for i, (traj_id, x, y, t, dist) in enumerate(unique_neighbors):
        print(f"{i+1}. trajectory_id={traj_id}, lon={x:.4f}, lat={y:.4f}, t={t}, distance={dist:.4f}")

# -----------------------------
# Run example query
# -----------------------------
query_knn(query_idx=0)  # Pick the first point for demonstration

