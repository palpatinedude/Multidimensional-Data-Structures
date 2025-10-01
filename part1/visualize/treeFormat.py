import json
import networkx as nx
import plotly.graph_objects as go
import matplotlib.pyplot as plt
import uuid

# Load the R-tree from JSON
with open("../rtree_output.json", "r") as f:
    rtree = json.load(f)

# --- NETWORKX TREE STRUCTURE ---
G = nx.DiGraph()

def add_to_networkx(node, parent=None):
    node_id = str(uuid.uuid4())
    label = "Leaf" if node.get("isLeaf") else "Node"

    G.add_node(node_id, label=label)  # <- explicitly add the node here

    if parent:
        G.add_edge(parent, node_id)

    if node.get("isLeaf"):
        for entry in node.get("entries", []):
            entry_id = str(uuid.uuid4())
            G.add_node(entry_id, label="Entry")  # <- also explicitly add entries
            G.add_edge(node_id, entry_id)
    else:
        for child in node.get("children", []):
            add_to_networkx(child["child"], node_id)


add_to_networkx(rtree)

# Draw tree
plt.figure(figsize=(12, 8))
pos = nx.spring_layout(G, seed=42)
labels = nx.get_node_attributes(G, "label")
nx.draw(G, pos, with_labels=False, arrows=True, node_size=500, node_color="skyblue")
nx.draw_networkx_labels(G, pos, labels, font_size=8)
plt.title("R-Tree Structure (NetworkX)")
plt.show()

# --- 3D PLOTLY VISUALIZATION ---
points_t = []
points_x = []
points_y = []

def extract_leaf_points(node):
    if node.get("isLeaf"):
        for entry in node.get("entries", []):
            pt = entry["point"]
            points_t.append(pt["t"])
            points_x.append(pt["x"])
            points_y.append(pt["y"])
    else:
        for child in node.get("children", []):
            extract_leaf_points(child["child"])

extract_leaf_points(rtree)

fig = go.Figure(data=[go.Scatter3d(
    x=points_x,
    y=points_y,
    z=points_t,
    mode='markers',
    marker=dict(
        size=4,
        color=points_t,
        colorscale='Viridis',
        opacity=0.8
    )
)])
fig.update_layout(
    title='3D R-Tree Points (x, y, t)',
    scene=dict(
        xaxis_title='X',
        yaxis_title='Y',
        zaxis_title='Time (T)'
    )
)
fig.show()
