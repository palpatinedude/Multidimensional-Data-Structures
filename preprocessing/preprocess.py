"""
preprocess.py

This script preprocesses the CityTrek-14K dataset, converting raw CSV trajectory 
data into compressed Parquet files for efficient processing. It then computes 
bounding boxes and timestamps for each trip and outputs a long-format Parquet file 
with all trajectories, ready for analysis.

Dataset Description:
CityTrek-14K contains 14,000 driving trajectories from 280 drivers (50 per driver) 
across Philadelphia, Atlanta, and Memphis. Data includes timestamps, vehicle speeds, 
and GPS coordinates at 1Hz. The dataset is anonymized for privacy. It has:
1. Summary file: contains trip-level information (trip ID, driver ID).
2. Trajectories file: detailed second-by-second GPS records (trip ID, timestamp, latitude, longitude).
*************** LINK  --> https://www.kaggle.com/datasets/sobhanmoosavi/citytrek-14k

This preprocessing script:
- Converts CSV trajectory chunks to Parquet for memory-efficient processing.
- Maps string IDs (drivers, trips) to sequential integers.
- Computes bounding boxes and start/end timestamps for each trip.
- Outputs a long-format Parquet file containing all trajectories with bounding box info.
- Monitors memory usage throughout the pipeline for efficiency tracking.
"""


import os
import time
import psutil
import pandas as pd
import dask.dataframe as dd

# File paths
SUMMARY_FILE = "../../summary_to_publish.csv"
TRAJECTORIES_FILE = "../../trajectories_to_publish.csv"
PARQUET_DIR = "trajectory_data_parquet"
OUTPUT_PARQUET = "trajectories_grouped.parquet"  # Final output in Parquet
CHUNK_SIZE = 10000
MB_DIVISOR = 1024 ** 2

# -----------------------------
# Utility function to monitor memory usage
# -----------------------------
def monitor_memory(tag=""):
    mem = psutil.Process(os.getpid()).memory_info().rss / MB_DIVISOR
    print(f"{tag} - Memory usage: {mem:.2f} MB")

# -----------------------------
# Mapping function
# -----------------------------
def convert_to_integer_mapping(series):
    unique_vals = sorted(series.unique())
    mapping = {val: i + 1 for i, val in enumerate(unique_vals)}
    return series.map(mapping), mapping

# -----------------------------
# CSV -> Parquet preprocessing
# -----------------------------
def preprocess_and_write_parquet(summary_file, traj_file, parquet_dir, chunk_size=CHUNK_SIZE):
    print("Step 1: Converting CSV to Parquet")
    start = time.time()
    os.makedirs(parquet_dir, exist_ok=True)

    # Read summary and map IDs
    summary = pd.read_csv(summary_file, usecols=["trip_id", "driver_id"])
    summary["driver_id"], driver_map = convert_to_integer_mapping(summary["driver_id"])
    summary["trip_id"], trip_map = convert_to_integer_mapping(summary["trip_id"])

    traj_iter = pd.read_csv(traj_file, usecols=["trip_id", "timestamp", "latitude", "longitude"], chunksize=chunk_size)
    file_counter = 0
    total_rows = 0

    for chunk in traj_iter:
        chunk["trip_id"] = chunk["trip_id"].map(trip_map).dropna().astype(int)
        merged = pd.merge(chunk, summary, on="trip_id", how="inner")
        merged["timestamp"] = pd.to_datetime(merged["timestamp"], format="%Y-%m-%d %H:%M:%S", errors="coerce")
        merged = merged.dropna(subset=["timestamp", "latitude", "longitude"])

        # Convert timestamp to seconds since epoch
        merged["t"] = (merged["timestamp"].astype("int64") // 1_000_000_000).astype("int64")


        df_out = merged[["driver_id", "trip_id", "longitude", "latitude", "t"]].rename(
            columns={
                "driver_id": "vehicle_id",
                "longitude": "x",
                "latitude": "y",
            }
        ).astype({
            "vehicle_id": "int32",
            "trip_id": "int32",
            "x": "float32",
            "y": "float32",
            "t": "int64",
        })

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
    print("Step 2: Loading Parquet files with Dask")
    ddf = dd.read_parquet(parquet_dir, engine="pyarrow")
    return ddf

# -----------------------------
# Compute bounding boxes and write long-format Parquet
# -----------------------------
def write_grouped_parquet_with_bbox(ddf, output_parquet):
    print("Step 3: Computing bounding boxes and writing Parquet")

    # Compute bbox per trip
    bbox_df = ddf.groupby(["vehicle_id", "trip_id"]).agg({
        "x": ["min", "max"],
        "y": ["min", "max"],
        "t": ["min", "max"]
    }).compute()
    bbox_df.columns = ["x_min", "x_max", "y_min", "y_max", "t_start", "t_end"]
    bbox_df = bbox_df.reset_index()

    # Merge bbox info back to long-format
    ddf = ddf.merge(bbox_df, on=["vehicle_id", "trip_id"], how="left")

    # Sort by vehicle, trip, time
    ddf = ddf.map_partitions(lambda df: df.sort_values(["vehicle_id", "trip_id", "t"]))

    # Write to Parquet
    ddf.to_parquet(output_parquet, engine="pyarrow", compression="snappy", write_index=False)
    print(f"Grouped Parquet with bounding boxes written: {output_parquet}")
    return ddf.shape[0].compute()

# -----------------------------
# Summarize dataset
# -----------------------------
def summarize_dataset(ddf, total_rows, parquet_rows_written):
    print("Summary Statistics")
    num_vehicles = ddf["vehicle_id"].nunique().compute()
    num_trips = ddf["trip_id"].nunique().compute()
    t_min = ddf["t"].min().compute()
    t_max = ddf["t"].max().compute()
    mem_footprint = ddf.memory_usage(deep=True).sum().compute() / MB_DIVISOR

    print(f"Total rows processed: {total_rows:,}")
    print(f"Total rows in Parquet: {parquet_rows_written:,}")
    print(f"Unique vehicles: {num_vehicles:,}")
    print(f"Unique trips: {num_trips:,}")
    print(f"Time range: {t_min} to {t_max}")
    print(f"Estimated memory footprint: {mem_footprint:.2f} MB")

# -----------------------------
# Main
# -----------------------------
def main():
    total_start = time.time()
    monitor_memory("Start")

    driver_map, trip_map, total_rows = preprocess_and_write_parquet(
        SUMMARY_FILE, TRAJECTORIES_FILE, PARQUET_DIR
    )
    monitor_memory("After CSV -> Parquet")

    ddf = load_parquet(PARQUET_DIR)
    monitor_memory("After Loading Parquet")

    parquet_rows_written = write_grouped_parquet_with_bbox(ddf, OUTPUT_PARQUET)
    monitor_memory("After Writing Parquet")

    summarize_dataset(ddf, total_rows, parquet_rows_written)

    total_end = time.time()
    print(f"Total processing time: {total_end - total_start:.2f} seconds")

# -----------------------------
# Execute
# -----------------------------
if __name__ == "__main__":
    main()
