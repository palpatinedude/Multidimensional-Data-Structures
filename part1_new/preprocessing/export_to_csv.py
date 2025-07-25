import dask.dataframe as dd
import pandas as pd

PARQUET_PATH = "trajectory_data_parquet"
CSV_OUTPUT = "trajectory_data.csv"

# Read the Parquet data
ddf = dd.read_parquet(PARQUET_PATH, engine="pyarrow")

# Select only the necessary columns
ddf = ddf[["vehicle_id", "trip_id", "x", "y", "t"]]

# Write to CSV (you can change `single_file=True` if you want one file only)
ddf.to_csv(CSV_OUTPUT, single_file=True, index=False)

df = pd.read_csv(CSV_OUTPUT)

print(df.head(60000))

# Print how many rows were written
print(f"Total rows written to {CSV_OUTPUT}: {len(df)}")

