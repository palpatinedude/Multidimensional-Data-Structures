import dask.dataframe as dd
import pandas as pd
import ijson
import time
import psutil
from sklearn.preprocessing import MinMaxScaler

INPUT_JSON = "processed_trajectories4.json"
PARQUET_OUTPUT = "trajectory_data_parquet"


def monitor_memory():
    mem_usage = psutil.Process().memory_info().rss / (1024 ** 2)
    print(f"Memory usage: {mem_usage:.2f} MB")

def process_json_with_ijson(input_file, chunk_size=50000):
    print("Streaming and processing JSON with ijson...")

    vehicle_ids = set()
    trip_ids = set()
    total_points = 0
    buffer = []
    chunks = []

    with open(input_file, "r") as f:
        drivers = ijson.items(f, "item")

        for driver in drivers:
            vehicle_id = driver["driver_id"]
            vehicle_ids.add(vehicle_id)

            for trip in driver["trips"]:
                trip_id = trip["trip_id"]
                trip_ids.add(trip_id)

                for point in trip["trajectory"]:
                    total_points += 1
                    buffer.append([
                        vehicle_id,
                        trip_id,
                        round(point["x"], 6),
                        round(point["y"], 6),
                        int(point["z"]) // 1000  # Convert ms to seconds
                    ])

                    if len(buffer) >= chunk_size:
                        df = pd.DataFrame(buffer, columns=["vehicle_id", "trip_id", "x", "y", "t"])
                        df = df.astype({
                            "vehicle_id": "int32", "trip_id": "int32",
                            "x": "float32", "y": "float32", "t": "int32"
                        })
                        chunks.append(df)
                        buffer = []

    # Final chunk
    if buffer:
        df = pd.DataFrame(buffer, columns=["vehicle_id", "trip_id", "x", "y", "t"])
        df = df.astype({
            "vehicle_id": "int32", "trip_id": "int32",
            "x": "float32", "y": "float32", "t": "int32"
        })
        chunks.append(df)

    print(f"Loaded {len(chunks)} chunks")

    # Combine all chunks into a Dask DataFrame
    ddf = dd.from_pandas(pd.concat(chunks, ignore_index=True), npartitions=len(chunks))

    # Normalize x and y (not t!) using MinMaxScaler
    def normalize_partition(pdf):
        scaler = MinMaxScaler()
        xy_scaled = scaler.fit_transform(pdf[["x", "y"]])
        pdf = pdf.copy()  # Optional, but safe
        pdf.loc[:, "x"] = xy_scaled[:, 0].astype("float32")
        pdf.loc[:, "y"] = xy_scaled[:, 1].astype("float32")
        return pdf

    ddf = ddf.map_partitions(normalize_partition, meta=ddf)

    # Persist to optimize memory
    ddf = ddf.persist()

    # Save to Parquet partitioned by vehicle_id
    ddf.to_parquet(
        PARQUET_OUTPUT,
        engine="pyarrow",
        compression="snappy",
        partition_on=["vehicle_id"]
    )

    print(f"Total points processed: {total_points}")
    print(f"Unique vehicles: {len(vehicle_ids)}")
    print(f"Unique trips: {len(trip_ids)}")

    return PARQUET_OUTPUT

def print_first_15_rows(parquet_path):
    print("\nPreviewing first 15 rows from saved Parquet data:\n")
    df = dd.read_parquet(parquet_path, engine="pyarrow")
    print(df.head(3000))

if __name__ == "__main__":
    monitor_memory()
    start = time.time()
    parquet_path = process_json_with_ijson(INPUT_JSON)
    end = time.time()
    print(f"Processing took {end - start:.2f} seconds")
    monitor_memory()

    print_first_15_rows(parquet_path)
