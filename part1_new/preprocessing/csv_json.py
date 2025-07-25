import pandas as pd
import json

CSV_PATH = "trajectory_data.csv"
JSON_OUTPUT = "trajectories_grouped.json"

df = pd.read_csv(CSV_PATH)

grouped = df.groupby(["vehicle_id", "trip_id"])

trajectory_list = []

for (veh_id, trip_id), group in grouped:
    points = group.sort_values("t")[["x", "y", "t"]].to_dict(orient="records")
    trajectory_list.append({
        "id": f"{veh_id}_{trip_id}",
        "points": points
    })

with open(JSON_OUTPUT, "w") as f:
    json.dump(trajectory_list, f)
