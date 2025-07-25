import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.dates as mdates

# Load and sample your data
CSV_PATH = "../../../data/trajectory_data.csv" 
df = pd.read_csv(CSV_PATH)
sampled_df = df.sample(n=5000, random_state=42)

# Convert t to datetime
sampled_df["t_dt"] = pd.to_datetime(sampled_df["t"], unit="s")

# Extract x, y, t
x = sampled_df['x']
y = sampled_df['y']
t = sampled_df['t_dt']  # now a datetime

# Plot
fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')

# Convert datetime to matplotlib's internal float format for plotting
t_numeric = mdates.date2num(t)

sc = ax.scatter(x, y, t_numeric, c=t_numeric, cmap='viridis', s=5)

# Format Z axis as datetime
ax.set_zlabel('Time')
ax.zaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d\n%H:%M:%S'))

# Axis labels
ax.set_xlabel('X (Normalized Longitude)')
ax.set_ylabel('Y (Normalized Latitude)')
plt.title("3D Trajectory Visualization (x, y, timestamp)")
plt.colorbar(sc, label='Time')
plt.tight_layout()
plt.show()
