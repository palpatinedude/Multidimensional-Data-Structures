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
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

# --- Draw a 3D box ---
def draw_box(ax, box, color='gray', alpha=0.2):
    if not box:
        return
    x0, x1 = box.get("minX",0), box.get("maxX",0)
    y0, y1 = box.get("minY",0), box.get("maxY",0)
    z0, z1 = box.get("minT",0), box.get("maxT",0)

    # 8 vertices
    vertices = [
        [x0, y0, z0], [x1, y0, z0], [x1, y1, z0], [x0, y1, z0],
        [x0, y0, z1], [x1, y0, z1], [x1, y1, z1], [x0, y1, z1]
    ]

    faces = [
        [vertices[i] for i in [0,1,2,3]],
        [vertices[i] for i in [4,5,6,7]],
        [vertices[i] for i in [0,1,5,4]],
        [vertices[i] for i in [2,3,7,6]],
        [vertices[i] for i in [1,2,6,5]],
        [vertices[i] for i in [0,3,7,4]]
    ]

    poly = Poly3DCollection(faces, facecolors=color, linewidths=0.5, edgecolors='k', alpha=alpha)
    ax.add_collection3d(poly)

# --- Recursive traversal ---
def draw_node_3d(ax, node, depth=0, max_depth=2, leaf_aggregate=True):
    if not node or depth > max_depth:
        return

    if node.get("isLeaf", False):
        # Aggregate leaf entries
        if leaf_aggregate and node.get("entries"):
            minX = min(e["box"]["minX"] for e in node["entries"])
            maxX = max(e["box"]["maxX"] for e in node["entries"])
            minY = min(e["box"]["minY"] for e in node["entries"])
            maxY = max(e["box"]["maxY"] for e in node["entries"])
            minT = min(e["box"]["minT"] for e in node["entries"])
            maxT = max(e["box"]["maxT"] for e in node["entries"])
            draw_box(ax, {"minX": minX, "maxX": maxX, "minY": minY, "maxY": maxY, "minT": minT, "maxT": maxT},
                     color='blue', alpha=0.3)
        else:
            # Draw each leaf entry individually (not recommended for huge trees)
            for e in node.get("entries", []):
                draw_box(ax, e["box"], color='green', alpha=0.2)
    else:
        # Draw this internal node
        box = node.get("box")
        if box:
            draw_box(ax, box, color='gray', alpha=0.2)
        for child in node.get("children", []):
            child_node = child.get("node") or child
            draw_node_3d(ax, child_node, depth+1, max_depth, leaf_aggregate)

# --- Main visualization ---
def visualize_rtree_3d(json_path="../results/bulkloaded_tree.json"):
    with open(json_path) as f:
        tree = json.load(f)

    fig = plt.figure(figsize=(12,10))
    ax = fig.add_subplot(111, projection='3d')

    draw_node_3d(ax, tree, max_depth=2, leaf_aggregate=True)

    ax.set_title("3D R-tree Visualization ")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("T (time)")
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    visualize_rtree_3d()
