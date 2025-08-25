"""
preprocess.py

This script preprocesses the CityTrek-14K dataset, converting raw CSV trajectory 
data into compressed Parquet files for efficient processing, and then creates 
a JSON file containing grouped trajectories with bounding boxes and timestamp info.

Dataset Description:
CityTrek-14K contains 14,000 driving trajectories from 280 drivers (50 per driver) 
across Philadelphia, Atlanta, and Memphis. Data includes timestamps, vehicle speeds, 
and GPS coordinates at 1Hz. The dataset is anonymized for privacy. It has:
1. Summary file: contains trip-level information.
2. Trajectories file: detailed second-by-second GPS records.

*************** LINK  --> https://www.kaggle.com/datasets/sobhanmoosavi/citytrek-14k

This preprocessing script:
- Converts CSVs to Parquet in chunks.
- Maps string IDs to sequential integers for easier processing.
- Computes bounding boxes for trajectories.
- Outputs a JSON file with all trajectories grouped by vehicle and trip.
"""

import os
import json
import time
import psutil
import pandas as pd
import dask.dataframe as dd

# File paths
SUMMARY_FILE = "../../summary_to_publish.csv"  # CSV containing trip_id and driver_id
TRAJECTORIES_FILE = "../../trajectories_to_publish.csv"  # CSV with GPS trajectories
PARQUET_DIR = "trajectory_data_parquet"  # Directory to store intermediate Parquet files
OUTPUT_JSON = "trajectories_grouped.json"  # Final grouped JSON output

CHUNK_SIZE = 10000  # Rows per CSV chunk to read
MB_DIVISOR = 1024 ** 2  # Convert bytes to MB for memory reporting

# -----------------------------
# Utility function to monitor memory usage
# -----------------------------
def monitor_memory(tag=""):
    """
    Prints current memory usage of the script in MB.
    """
    mem = psutil.Process(os.getpid()).memory_info().rss / MB_DIVISOR
    print(f"{tag} - Memory usage: {mem:.2f} MB")

# -----------------------------
# Mapping function
# -----------------------------
def convert_to_integer_mapping(series):
    """
    Maps unique string IDs to sequential integers starting from 1.
    Returns a transformed series and the mapping dictionary.
    """
    unique_vals = sorted(series.unique())
    mapping = {val: i + 1 for i, val in enumerate(unique_vals)}
    return series.map(mapping), mapping

# -----------------------------
# CSV -> Parquet preprocessing
# -----------------------------
def preprocess_and_write_parquet(summary_file, traj_file, parquet_dir, chunk_size=CHUNK_SIZE):
    """
    Reads summary and trajectory CSVs in chunks, maps string IDs to integers, 
    cleans data, and writes them to compressed Parquet files for efficient storage.
    """
    print("Step 1: Converting CSV to Parquet ")
    start = time.time()
    os.makedirs(parquet_dir, exist_ok=True)

    # Read summary file to get trip_id <-> driver_id mapping
    summary = pd.read_csv(summary_file, usecols=["trip_id", "driver_id"])
    summary["driver_id"], driver_map = convert_to_integer_mapping(summary["driver_id"])
    summary["trip_id"], trip_map = convert_to_integer_mapping(summary["trip_id"])

    # Read trajectory CSV in chunks
    traj_iter = pd.read_csv(traj_file, usecols=["trip_id", "timestamp", "latitude", "longitude"], chunksize=chunk_size)
    file_counter = 0
    total_rows = 0

    for chunk in traj_iter:
        # Convert trip_id to integer
        chunk["trip_id"] = chunk["trip_id"].map(trip_map).dropna().astype(int)
        # Merge with summary to get driver_id
        merged = pd.merge(chunk, summary, on="trip_id", how="inner")
        # Convert timestamp column to datetime
        merged["timestamp"] = pd.to_datetime(merged["timestamp"], format="%Y-%m-%d %H:%M:%S", errors="coerce")
        # Drop rows with invalid data
        merged = merged.dropna(subset=["timestamp", "latitude", "longitude"])
        # Rename and reorder columns
        df_out = merged[["driver_id", "trip_id", "longitude", "latitude", "timestamp"]].rename(
            columns={
                "driver_id": "vehicle_id",
                "longitude": "x",
                "latitude": "y",
                "timestamp": "t"
            }
        ).astype({
            "vehicle_id": "int32",
            "trip_id": "int32",
            "x": "float32",
            "y": "float32",
        })
        # Write each chunk to Parquet
        df_out.to_parquet(
            os.path.join(parquet_dir, f"chunk_{file_counter}.parquet"),
            engine="pyarrow",
            compression="snappy"
        )
        file_counter += 1
        total_rows += len(df_out)

    duration = time.time() - start
    print(f"CSV -> Parquet complete: {total_rows:,} rows, {file_counter} chunks, Time: {duration:.2f}s")
    return driver_map, trip_map, total_rows

# -----------------------------
# Load Parquet data
# -----------------------------
def load_parquet(parquet_dir):
    """
    Loads preprocessed Parquet files into a Dask dataframe for further processing.
    """
    print("Step 2: Loading Parquet files with Dask")
    ddf = dd.read_parquet(parquet_dir, engine="pyarrow")
    return ddf

# -----------------------------
# Generate grouped JSON
# -----------------------------
def write_grouped_json_from_dask(ddf, output_json):
    """
    Groups trajectories by vehicle_id and trip_id, computes bounding boxes, 
    and writes JSON output containing points and bbox information.
    """
    print("Step 3: Grouping, computing bounding boxes, writing JSON")
    ddf = ddf[["vehicle_id", "trip_id", "x", "y", "t"]]
    # Sort trajectories by time per trip
    ddf = ddf.map_partitions(lambda df: df.sort_values(["vehicle_id", "trip_id", "t"]))

    # Build JSON objects partition by partition
    def build_json_partition(df):
        results = []
        for (vehicle_id, trip_id), group in df.groupby(["vehicle_id", "trip_id"], sort=False):
            # Bounding box (x_min, y_min, t_start, x_max, y_max, t_end)
            x_min = f"{group['x'].min():.2f}"
            x_max = f"{group['x'].max():.2f}"
            y_min = f"{group['y'].min():.2f}"
            y_max = f"{group['y'].max():.2f}"
            t_start, t_end = group["t"].min(), group["t"].max()

            # Round points and format as string
            points = group[["x", "y", "t"]].copy()
            points["x"] = points["x"].round(2).map(lambda v: f"{v:.2f}")
            points["y"] = points["y"].round(2).map(lambda v: f"{v:.2f}")
            points["t"] = points["t"].dt.strftime("%Y-%m-%dT%H:%M:%S")
            points = points.to_dict(orient="records")

            bbox = [
                x_min,
                y_min,
                t_start.strftime("%Y-%m-%dT%H:%M:%S"),
                x_max,
                y_max,
                t_end.strftime("%Y-%m-%dT%H:%M:%S")
            ]

            results.append({
                "id": f"{vehicle_id}_{trip_id}",
                "bbox": bbox,
                "points": points
            })
        return pd.DataFrame({"json": results})

    json_df = ddf.map_partitions(build_json_partition, meta={"json": "object"}).compute()
    # Write final JSON file
    with open(output_json, "w") as f:
        json.dump(list(json_df["json"]), f, indent=2)
    print(f"JSON written: {output_json}")
    return len(json_df)

# -----------------------------
# Summarize dataset statistics
# -----------------------------
def summarize_dataset(ddf, total_rows, json_objects_written):
    """
    Prints statistics about the dataset including number of vehicles, trips, 
    time range, and estimated memory usage.
    """
    print("Summary Statistics")
    num_vehicles = ddf["vehicle_id"].nunique().compute()
    num_trips = ddf["trip_id"].nunique().compute()
    t_min = ddf["t"].min().compute()
    t_max = ddf["t"].max().compute()
    mem_footprint = ddf.memory_usage(deep=True).sum().compute() / MB_DIVISOR
    print(f"Total rows processed: {total_rows:,}")
    print(f"Total trajectory objects in JSON: {json_objects_written:,}")
    print(f"Unique vehicles: {num_vehicles:,}")
    print(f"Unique trips: {num_trips:,}")
    print(f"Time range: {t_min} to {t_max}")
    print(f"Estimated memory footprint: {mem_footprint:.2f} MB")

# -----------------------------
# Main function
# -----------------------------
def main():
    """
    Main orchestrator: preprocess CSV -> Parquet -> Load Parquet -> JSON -> Summary
    """
    total_start = time.time()
    monitor_memory("Start")
    
    driver_map, trip_map, total_rows = preprocess_and_write_parquet(
        SUMMARY_FILE, TRAJECTORIES_FILE, PARQUET_DIR
    )
    monitor_memory("After CSV --> Parquet")
    
    ddf = load_parquet(PARQUET_DIR)
    monitor_memory("After Loading Parquet")
    
    json_objects_written = write_grouped_json_from_dask(ddf, OUTPUT_JSON)
    monitor_memory("After JSON Output")
    
    summarize_dataset(ddf, total_rows, json_objects_written)
    total_end = time.time()
    print(f"Total processing time: {total_end - total_start:.2f} seconds")

# -----------------------------
# Execute script
# -----------------------------
if __name__ == "__main__":
    main()
