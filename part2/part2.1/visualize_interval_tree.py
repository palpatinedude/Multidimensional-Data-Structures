
import numpy as np
import matplotlib.pyplot as plt
import json
import plotly.graph_objects as go
import networkx as nx

# Function to create bounding box wireframe
def create_bounding_box(minX, minY, minZ, maxX, maxY, maxZ):
    x = [minX, maxX, maxX, minX, minX, maxX, maxX, minX]
    y = [minY, minY, maxY, maxY, minY, minY, maxY, maxY]
    z = [minZ, minZ, minZ, minZ, maxZ, maxZ, maxZ, maxZ]
    lines = [
        [0, 1], [1, 2], [2, 3], [3, 0],  # Bottom square
        [4, 5], [5, 6], [6, 7], [7, 4],  # Top square
        [0, 4], [1, 5], [2, 6], [3, 7]   # Vertical lines
    ]
    line_x, line_y, line_z = [], [], []
    for line in lines:
        line_x.append([x[line[0]], x[line[1]]])
        line_y.append([y[line[0]], y[line[1]]])
        line_z.append([z[line[0]], z[line[1]]])
    return line_x, line_y, line_z

# Function to create 3D scatter plot for trajectory points
def create_trajectory_points(trajectory):
    x = [point['x'] for point in trajectory]
    y = [point['y'] for point in trajectory]
    z = [point['z'] for point in trajectory]
    return x, y, z

# Recursive function to count bounding boxes
def count_bounding_boxes(node):
    count = 1  # Count the current node
    if "children" in node:
        for child in node["children"]:
            count += count_bounding_boxes(child)
    return count

# Recursive function to process and plot bounding boxes and points
def process_node(node, fig):
    bounding_box = node["boundingBox"]
    minX, minY, minZ = bounding_box["minX"], bounding_box["minY"], bounding_box["minZ"]
    maxX, maxY, maxZ = bounding_box["maxX"], bounding_box["maxY"], bounding_box["maxZ"]
    line_x, line_y, line_z = create_bounding_box(minX, minY, minZ, maxX, maxY, maxZ)
    for i in range(len(line_x)):
        fig.add_trace(go.Scatter3d(
            x=line_x[i], y=line_y[i], z=line_z[i],
            mode='lines',
            line=dict(color='blue' if "children" in node else 'red')
        ))
    if "isLeaf" in node and node["isLeaf"]:
        if "entries" in node:
            for entry in node["entries"]:
                if "trajectory" in entry:
                    x, y, z = create_trajectory_points(entry["trajectory"])
                    fig.add_trace(go.Scatter3d(
                        x=x, y=y, z=z,
                        mode='markers',
                        marker=dict(color='orange', size=5)
                    ))
    if "children" in node:
        for child in node["children"]:
            process_node(child, fig)

# Function to visualize the R-tree structure with NetworkX
def build_rtree_graph(rtree_data):
    G = nx.DiGraph()
    node_labels = {}
    node_colors = []
    node_counter = {'internal': 1, 'leaf': 1}

    def add_nodes_edges(node, parent_id=None, depth=0):
        is_leaf = node.get('isLeaf', False)
        node_id = id(node)
        label = f"Leaf{node_counter['leaf']}" if is_leaf else f"Box{node_counter['internal']}"
        if is_leaf:
            node_counter['leaf'] += 1
        else:
            node_counter['internal'] += 1
        node_labels[node_id] = label
        node_colors.append('green' if is_leaf else 'blue')
        G.add_node(node_id, depth=depth, subset=depth)
        if parent_id is not None:
            G.add_edge(parent_id, node_id)
        for child in node.get('children', []):
            add_nodes_edges(child, node_id, depth + 1)
    add_nodes_edges(rtree_data)
    return G, node_labels, node_colors

def visualize_rtree_structure(rtree_data):
    G, node_labels, node_colors = build_rtree_graph(rtree_data)
    pos = nx.multipartite_layout(G, subset_key="subset", align="horizontal")
    pos = {node: (x, -y) for node, (x, y) in pos.items()}
    plt.figure(figsize=(12, 8))
    nx.draw(
        G, pos, labels=node_labels, with_labels=True,
        node_size=800, node_color=node_colors, font_size=10, font_weight='bold', arrows=True
    )
    plt.title("R-tree Structure")
    plt.show()

# File Handling Functions
def load_json(file_path):
    with open(file_path, 'r') as file:
        return json.load(file)

# Main Entry Point
def main():
    file_path = 'main/tree1_output.json'  
    data = load_json(file_path)
    
    # Count and print the total number of bounding boxes
    total_bounding_boxes = count_bounding_boxes(data)
    print(f"Total number of bounding boxes: {total_bounding_boxes}")
    
    # Visualize the 3D structure with Plotly
    fig = go.Figure()
    process_node(data, fig)
    fig.update_layout(scene=dict(
        xaxis_title='X',
        yaxis_title='Y',
        zaxis_title='Z',
    ),
        title="3D Visualization of R-tree Bounding Boxes and Points",
        showlegend=False
    )
    fig.show()
    
    # Visualize the tree structure with NetworkX
    visualize_rtree_structure(data)

if __name__ == "__main__":
    main()