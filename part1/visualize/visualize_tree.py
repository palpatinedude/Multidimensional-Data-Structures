''' 2D visualization of an R-tree structure with trajectories.
import json
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import random
import os

# --- Color utility ---
def random_color():
    return (random.random(), random.random(), random.random())

# --- Draw a bounding box rectangle ---
def draw_mbr(ax, mbr, edgecolor='black', linewidth=1.0, linestyle='--'):
    rect = patches.Rectangle(
        (mbr["minX"], mbr["minY"]),
        mbr["maxX"] - mbr["minX"],
        mbr["maxY"] - mbr["minY"],
        linewidth=linewidth,
        edgecolor=edgecolor,
        facecolor='none',
        linestyle=linestyle
    )
    ax.add_patch(rect)

# --- Draw a trajectory (list of points) ---
def draw_trajectory(ax, traj):
    color = random_color()
    xs = [pt["x"] for pt in traj["points"]]
    ys = [pt["y"] for pt in traj["points"]]
    ax.plot(xs, ys, marker='o', label=traj["id"], color=color)

# --- Recursive tree traversal ---
def draw_node(ax, node, depth=0):
    draw_mbr(ax, node["mbr"], edgecolor='black', linewidth=1 + 0.5 * (2 - depth), linestyle='--')

    if node["type"] == "leaf":
        for traj in node.get("trajectories", []):
            draw_trajectory(ax, traj)
    else:
        for child in node.get("children", []):
            draw_node(ax, child, depth + 1)

# --- Main function ---
def visualize_rtree(json_path="../rtree_output_test.json"):
    if not os.path.exists(json_path):
        print(f"Error: JSON file not found at {json_path}")
        return

    with open(json_path, "r") as f:
        tree = json.load(f)

    fig, ax = plt.subplots(figsize=(10, 8))
    draw_node(ax, tree)
    ax.set_title("R-tree Visualization")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_aspect('equal')
    ax.legend()
    plt.grid(True)
    plt.show()

# --- Run script directly ---
if __name__ == "__main__":
    visualize_rtree()
'''
import json
import matplotlib.pyplot as plt
import random
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

# --- Color utility ---
def random_color():
    return (random.random(), random.random(), random.random())

# --- Draw a 3D box (MBR) ---
def draw_3d_mbr(ax, mbr, color='gray', alpha=0.2):
    # 8 corners of the box
    x0, x1 = mbr["minX"], mbr["maxX"]
    y0, y1 = mbr["minY"], mbr["maxY"]
    z0, z1 = mbr["minT"], mbr["maxT"]

    # Vertices
    vertices = [
        [x0, y0, z0], [x1, y0, z0], [x1, y1, z0], [x0, y1, z0],
        [x0, y0, z1], [x1, y0, z1], [x1, y1, z1], [x0, y1, z1]
    ]

    # Define 6 faces using vertex indices
    faces = [
        [vertices[i] for i in [0, 1, 2, 3]],  # bottom
        [vertices[i] for i in [4, 5, 6, 7]],  # top
        [vertices[i] for i in [0, 1, 5, 4]],  # front
        [vertices[i] for i in [2, 3, 7, 6]],  # back
        [vertices[i] for i in [1, 2, 6, 5]],  # right
        [vertices[i] for i in [0, 3, 7, 4]]   # left
    ]

    box = Poly3DCollection(faces, facecolors=color, linewidths=0.5, edgecolors='k', alpha=alpha)
    ax.add_collection3d(box)

# --- Draw trajectory in 3D ---
def draw_trajectory(ax, traj):
    color = random_color()
    xs = [p["x"] for p in traj["points"]]
    ys = [p["y"] for p in traj["points"]]
    zs = [p["t"] for p in traj["points"]]
    ax.plot(xs, ys, zs, marker='o', label=traj["id"], color=color)

# --- Recursive node traversal ---
def draw_node_3d(ax, node, depth=0):
    draw_3d_mbr(ax, node["mbr"], color='blue' if node["type"] == "leaf" else 'gray', alpha=0.2)

    if node["type"] == "leaf":
        for traj in node.get("trajectories", []):
            draw_trajectory(ax, traj)
    else:
        for child in node.get("children", []):
            draw_node_3d(ax, child, depth + 1)

# --- Main function ---
def visualize_rtree_3d(json_path="../rtree_output_test.json"):
    with open(json_path) as f:
        tree = json.load(f)

    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection='3d')

    draw_node_3d(ax, tree)

    ax.set_title("3D R-tree Visualization")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Time (T)")
    ax.legend()
    plt.tight_layout()
    plt.show()

# --- Run directly ---
if __name__ == "__main__":
    visualize_rtree_3d()
