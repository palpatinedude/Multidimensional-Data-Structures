# This code reads trajectory data from a CSV file, separates a query trajectory 
# from its similar neighbor trajectories, and visualizes all trajectories 
# in a 3D interactive plot using Plotly.

import pandas as pd
import plotly.graph_objects as go
import plotly.express as px

# ---------------- Load CSV ----------------
df = pd.read_csv("../results/query_3_findSimilar_plot.csv") 

# ---------------- Separate trajectories ----------------
# Query trajectory
query_df = df[df['Type'] == 'query']

# Similar neighbor trajectories
result_df = df[df['Type'] == 'result']

# Get unique neighbor trajectory IDs
neighbor_ids = result_df['TrajectoryID'].unique()

# ---------------- Create 3D figure ----------------
fig = go.Figure()  # Initialize an empty 3D figure

# ---------------- Plot query trajectory ----------------
fig.add_trace(go.Scatter3d(
    x=query_df['X'],  # Longitude
    y=query_df['Y'],  # Latitude
    z=query_df['T'],  # Time
    mode='lines+markers',  # Show line with points
    name='Query Trajectory',  # Label in legend
    marker=dict(size=4, color='red'),  # Red markers
    line=dict(color='red', width=4),  # Red line
    text=query_df['TrajectoryID'],  # Hover text
    hovertemplate='TrajectoryID: %{text}<br>X: %{x}<br>Y: %{y}<br>T: %{z}'  # Hover info
))

# ---------------- Plot neighbor trajectories ----------------
colors = px.colors.qualitative.Dark24  # 24 distinct colors for neighbors
for i, traj_id in enumerate(neighbor_ids):
    traj_points = result_df[result_df['TrajectoryID'] == traj_id]  # Points for this neighbor
    fig.add_trace(go.Scatter3d(
        x=traj_points['X'],
        y=traj_points['Y'],
        z=traj_points['T'],
        mode='lines+markers',
        name=f'Neighbor {traj_id}',  # Label each neighbor
        marker=dict(size=3, color=colors[i % len(colors)]),
        line=dict(width=2, color=colors[i % len(colors)]),
        text=traj_points['TrajectoryID'],
        hovertemplate='TrajectoryID: %{text}<br>X: %{x}<br>Y: %{y}<br>T: %{z}'
    ))

# ---------------- Layout settings ----------------
fig.update_layout(
    title="Find Similar Neighbors Trajectories (3D)",
    scene=dict(
        xaxis_title='Longitude (X)',
        yaxis_title='Latitude (Y)',
        zaxis_title='Time (T)',
        xaxis=dict(showbackground=True),
        yaxis=dict(showbackground=True),
        zaxis=dict(showbackground=True),
    ),
    legend=dict(itemsizing='constant')  # Keep legend spacing constant
)

# ---------------- Show interactive plot ----------------
fig.show()
