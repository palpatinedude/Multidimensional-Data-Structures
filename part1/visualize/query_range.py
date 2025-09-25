# This code reads trajectory data from a CSV file from range query, groups the data by trajectory ID,
# and visualizes each trajectory in a 3D plot with optional bounding boxes using Plotly.

import pandas as pd
import plotly.graph_objects as go
import plotly.express as px



# ---------------- Load CSV ----------------
csv_file = "../results/query_1_rangeQuery_rtree.csv" 
df = pd.read_csv(csv_file)  # Read CSV into a pandas DataFrame

# ---------------- Group trajectories ----------------
# Create a dictionary where each key is a TrajectoryID and value is its data
trajectories = {tid: group for tid, group in df.groupby("TrajectoryID")}

# ---------------- Create Figure ----------------
fig = go.Figure()  # Initialize an empty 3D figure

# Colors for different trajectories
colors = px.colors.qualitative.Plotly  # Use Plotly's qualitative color palette
color_map = {tid: colors[i % len(colors)] for i, tid in enumerate(trajectories.keys())}

# ---------------- Plot each trajectory ----------------
for tid, traj in trajectories.items():
    # Add trajectory as a 3D line with markers
    fig.add_trace(go.Scatter3d(
        x=traj['X'],  # Longitude
        y=traj['Y'],  # Latitude
        z=traj['T'],  # Timestamp
        mode='lines+markers',  # Show lines and points
        line=dict(color=color_map[tid], width=4),
        marker=dict(size=3, color=color_map[tid]),
        name=tid,  # Trajectory ID
        text=traj['T'],  # Hover text
        hovertemplate="TrajectoryID: %{name}<br>Lon: %{x}<br>Lat: %{y}<br>Time: %{z}<extra></extra>"
    ))

    # Optional: Draw 3D bounding box around the trajectory
    minX, minY = traj['MinX'].iloc[0], traj['MinY'].iloc[0]
    maxX, maxY = traj['MaxX'].iloc[0], traj['MaxY'].iloc[0]
    minT, maxT = traj['T'].min(), traj['T'].max()

    # Define 12 edges of the box (bottom, top, and vertical edges)
    box_edges = [
        # Bottom rectangle (minT)
        ([minX, maxX], [minY, minY], [minT, minT]),
        ([maxX, maxX], [minY, maxY], [minT, minT]),
        ([maxX, minX], [maxY, maxY], [minT, minT]),
        ([minX, minX], [maxY, minY], [minT, minT]),
        # Top rectangle (maxT)
        ([minX, maxX], [minY, minY], [maxT, maxT]),
        ([maxX, maxX], [minY, maxY], [maxT, maxT]),
        ([maxX, minX], [maxY, maxY], [maxT, maxT]),
        ([minX, minX], [maxY, minY], [maxT, maxT]),
        # Vertical edges
        ([minX, minX], [minY, minY], [minT, maxT]),
        ([maxX, maxX], [minY, minY], [minT, maxT]),
        ([maxX, maxX], [maxY, maxY], [minT, maxT]),
        ([minX, minX], [maxY, maxY], [minT, maxT])
    ]

    # Add edges of the bounding box to the figure
    for edge in box_edges:
        fig.add_trace(go.Scatter3d(
            x=edge[0],
            y=edge[1],
            z=edge[2],
            mode='lines',
            line=dict(color='gray', width=1),
            showlegend=False  # Don't add box edges to legend
        ))

# ---------------- Layout ----------------
fig.update_layout(
    scene=dict(
        xaxis_title='Longitude',
        yaxis_title='Latitude',
        zaxis_title='Timestamp'
    ),
    title='3D Trajectory Visualization (RTree Results)',
    margin=dict(l=0, r=0, b=0, t=30)  # Reduce margins
)

# Display the 3D plot
fig.show()