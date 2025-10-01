"""
3D Convex Hull Visualization and Performance Analysis

This script loads 3D point sets and their convex hulls computed
by different algorithms (QuickHull3D or Chans 3D)
from the results folder, visualizes them in 3D, and optionally
plots measured time and memory usage from CSV results.

"""

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np
import glob
import pandas as pd
import os
import matplotlib.cm as cm

# -----------------------------
# CONFIG: set folder to algorithm results
folder = "../results/quickHull3D/"
#folder = "../results/chan3D/"

# -----------------------------

# -----------------------------
# Helper: Load 3D points from a text file
def load_points_3D(file_path):
    points = []
    with open(file_path) as f:
        for line in f:
            x, y, z = map(float, line.split())
            points.append((x, y, z))
    return np.array(points)

# Helper: Load faces (triangles) from a text file
def load_faces(file_path):
    faces = []
    with open(file_path) as f:
        for line in f:
            faces.append(list(map(int, line.split())))
    return faces

# Helper: Extract input size 'n' from filename
def extract_n(filename):
    base = os.path.basename(filename)
    parts = base.split('_')
    for part in parts:
        if part.isdigit() and int(part) > 1:
            return int(part)
    raise ValueError(f"Cannot extract n from filename: {filename}")

# -----------------------------
# Detect file type and mode
vertices_files = sorted(glob.glob(os.path.join(folder, "*_vertices_*.txt")))
faces_files    = sorted(glob.glob(os.path.join(folder, "*_faces_*.txt")))
points_files   = sorted(glob.glob(os.path.join(folder, "*_points_*.txt")))
hull_files     = sorted(glob.glob(os.path.join(folder, "*_hull_*.txt")))

if vertices_files and faces_files:
    mode = "faces"  # QuickHull3D style
elif points_files and hull_files:
    mode = "hull"   # Divide & Conquer 3D style
else:
    raise RuntimeError("No valid 3D data files found in folder")

# -----------------------------
# Consider only the first run per input size
unique_n = sorted(set(extract_n(f) for f in points_files))

# -----------------------------
# Plot each point set with its 3D hull
if mode == "faces":
    for n in unique_n:
        # Load vertices and faces
        pts_file = next(f for f in vertices_files if extract_n(f) == n)
        fcs_file = next(f for f in faces_files if extract_n(f) == n)
        verts = load_points_3D(pts_file)
        faces = load_faces(fcs_file)

        fig = plt.figure(figsize=(8,6))
        ax = fig.add_subplot(111, projection='3d')

        # Plot all vertices as grey points
        ax.scatter(verts[:,0], verts[:,1], verts[:,2], color='grey', alpha=0.6, label='Hull vertices')

        # Plot each triangular face
        cmap = cm.get_cmap('tab20', len(faces))
        for i, f in enumerate(faces):
            tri = [verts[f[0]], verts[f[1]], verts[f[2]]]
            poly3d = Poly3DCollection([tri], alpha=0.5, facecolor=cmap(i), edgecolor='k')
            ax.add_collection3d(poly3d)

        # Labels and title
        ax.set_title(f"3D Convex Hull: n={n}")
        ax.set_xlabel("X")
        ax.set_ylabel("Y")
        ax.set_zlabel("Z")
        plt.legend()
        plt.show()

else:  # mode == "hull"
    for n in unique_n:
        pts_file  = next(f for f in points_files if extract_n(f) == n)
        hull_file = next(f for f in hull_files if extract_n(f) == n)
        points = load_points_3D(pts_file)
        hull   = load_points_3D(hull_file)

        fig = plt.figure(figsize=(8,6))
        ax = fig.add_subplot(111, projection='3d')

        # Plot original points
        ax.scatter(points[:,0], points[:,1], points[:,2], color='grey', alpha=0.6, label='Points')

        # Plot hull triangles
        for i in range(0, len(hull), 3):
            if i+2 < len(hull):
                verts_tri = [hull[i], hull[i+1], hull[i+2]]
                poly3d = Poly3DCollection([verts_tri], alpha=0.5, facecolor='red', edgecolor='k')
                ax.add_collection3d(poly3d)

        ax.set_title(f"3D Convex Hull: n={n}")
        ax.set_xlabel("X")
        ax.set_ylabel("Y")
        ax.set_zlabel("Z")
        plt.legend()
        plt.show()

# -----------------------------
# Plot time and memory usage from CSV
csv_file = os.path.join(folder, os.path.basename(folder.rstrip('/')) + "_results.csv")
if os.path.exists(csv_file):
    df = pd.read_csv(csv_file)
    print("CSV loaded:", csv_file)
    print(df.head())

    n = df['n'].values

    # Check if CSV contains averages or single-run results
    if 'avg_time_us' in df.columns:
        time = df['avg_time_us'].values
        mem  = df['avg_mem_KB'].values
        std_time = df['std_time_us'].values
        std_mem  = df['std_mem_KB'].values
    else:
        time = df['time_us'].values
        mem  = df['approx_mem_KB'].values
        std_time = np.zeros_like(time)
        std_mem  = np.zeros_like(mem)

    plt.figure(figsize=(12,5))

    # ----------------- Time plot -----------------
    plt.subplot(1,2,1)
    if std_time.max() > 0:
        plt.errorbar(n, time, yerr=std_time, fmt='bo-', label='Measured time')
    else:
        plt.plot(n, time, 'bo-', label='Measured time')
    plt.plot(n, time[0]*(n*np.log2(n)/(n[0]*np.log2(n[0]))), 'r--', label='O(n log n) reference')
    plt.xlabel("Input size n")
    plt.ylabel("Time (us)")
    plt.title("3D Convex Hull: Time Complexity")
    plt.legend()
    plt.grid(True, which="both", ls="--")

    # ----------------- Memory plot -----------------
    plt.subplot(1,2,2)
    if std_mem.max() > 0:
        plt.errorbar(n, mem, yerr=std_mem, fmt='go-', label='Measured memory')
    else:
        plt.plot(n, mem, 'go-', label='Measured memory')
    plt.plot(n, mem[0]*(n/n[0]), 'r--', label='O(n) reference')
    plt.xlabel("Input size n")
    plt.ylabel("Memory (KB)")
    plt.title("3D Convex Hull: Memory Usage")
    plt.legend()
    plt.grid(True, which="both", ls="--")

    plt.tight_layout()
    plt.show()
"""
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np
import glob
import pandas as pd
import os
import matplotlib.cm as cm

# -----------------------------
# CONFIG: folder with algorithm results
folder = "../results/quickHull3D/"
#folder = "../results/chan3D/"

# -----------------------------
# Helper: Load 3D points from a text file
def load_points_3D(file_path):
    points = []
    with open(file_path) as f:
        for line in f:
            x, y, z = map(float, line.split())
            points.append((x, y, z))
    return np.array(points)

# Helper: Load faces (triangles) from a text file
def load_faces(file_path):
    faces = []
    with open(file_path) as f:
        for line in f:
            faces.append(list(map(int, line.split())))
    return faces

# Helper: Extract input size 'n' from filename
def extract_n(filename):
    base = os.path.basename(filename)
    parts = base.split('_')
    for part in parts:
        if part.isdigit() and int(part) > 1:
            return int(part)
    raise ValueError(f"Cannot extract n from filename: {filename}")

# -----------------------------
# Detect files
vertices_files = sorted(glob.glob(os.path.join(folder, "*_vertices_*.txt")))
faces_files    = sorted(glob.glob(os.path.join(folder, "*_faces_*.txt")))
points_files   = sorted(glob.glob(os.path.join(folder, "*_points_*.txt")))
hull_files     = sorted(glob.glob(os.path.join(folder, "*_hull_*.txt")))

# -----------------------------
# Determine plotting mode
if vertices_files and faces_files:
    mode = "faces"
elif points_files and hull_files:
    mode = "hull"
else:
    raise RuntimeError("No valid 3D data files found in folder")

# -----------------------------
# Unique input sizes
unique_n = sorted(set(extract_n(f) for f in points_files))

# -----------------------------
# Unified plotting function
def plot_3D_hull(points, vertices=None, faces=None, title="3D Convex Hull"):
    fig = plt.figure(figsize=(8,6))
    ax = fig.add_subplot(111, projection='3d')

    # Plot original points
    ax.scatter(points[:,0], points[:,1], points[:,2], color='grey', alpha=0.6, label='Points')

    # Plot faces if available
    if vertices is not None and faces is not None:
        cmap = cm.get_cmap('tab20', len(faces))
        for i, f in enumerate(faces):
            tri = [vertices[f[0]], vertices[f[1]], vertices[f[2]]]
            poly3d = Poly3DCollection([tri], alpha=0.5, facecolor=cmap(i), edgecolor='k')
            ax.add_collection3d(poly3d)
        ax.scatter(vertices[:,0], vertices[:,1], vertices[:,2], color='blue', alpha=0.5, label='Hull vertices')

    ax.set_title(title)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")
    plt.legend()
    plt.show()

# -----------------------------
# Plot each dataset
for n in unique_n:
    pts_file = next(f for f in points_files if extract_n(f) == n)
    points = load_points_3D(pts_file)

    if mode == "faces":
        verts_file = next(f for f in vertices_files if extract_n(f) == n)
        faces_file = next(f for f in faces_files if extract_n(f) == n)
        vertices = load_points_3D(verts_file)
        faces = load_faces(faces_file)
        plot_3D_hull(points, vertices, faces, title=f"3D Convex Hull: n={n}")
    else:
        hull_file = next(f for f in hull_files if extract_n(f) == n)
        hull = load_points_3D(hull_file)
        # Hull as triangles: group every 3 consecutive points
        faces = [ [i, i+1, i+2] for i in range(0, len(hull), 3) if i+2 < len(hull) ]
        plot_3D_hull(points, hull, faces, title=f"3D Convex Hull: n={n}")

# -----------------------------
# Plot time and memory from CSV
csv_file = os.path.join(folder, os.path.basename(folder.rstrip('/')) + "_results.csv")
if os.path.exists(csv_file):
    df = pd.read_csv(csv_file)
    print("CSV loaded:", csv_file)
    print(df.head())

    n = df['n'].values
    if 'avg_time_us' in df.columns:
        time = df['avg_time_us'].values
        mem  = df['avg_mem_KB'].values
        std_time = df['std_time_us'].values
        std_mem  = df['std_mem_KB'].values
    else:
        time = df['time_us'].values
        mem  = df['approx_mem_KB'].values
        std_time = np.zeros_like(time)
        std_mem  = np.zeros_like(mem)

    plt.figure(figsize=(12,5))

    # ----------------- Time plot -----------------
    plt.subplot(1,2,1)
    if std_time.max() > 0:
        plt.errorbar(n, time, yerr=std_time, fmt='bo-', label='Measured time')
    else:
        plt.plot(n, time, 'bo-', label='Measured time')
    plt.plot(n, time[0]*(n*np.log2(n)/(n[0]*np.log2(n[0]))), 'r--', label='O(n log n) reference')
    plt.xlabel("Input size n")
    plt.ylabel("Time (us)")
    plt.title("3D Convex Hull: Time Complexity")
    plt.legend()
    plt.grid(True, which="both", ls="--")

    # ----------------- Memory plot -----------------
    plt.subplot(1,2,2)
    if std_mem.max() > 0:
        plt.errorbar(n, mem, yerr=std_mem, fmt='go-', label='Measured memory')
    else:
        plt.plot(n, mem, 'go-', label='Measured memory')
    plt.plot(n, mem[0]*(n/n[0]), 'r--', label='O(n) reference')
    plt.xlabel("Input size n")
    plt.ylabel("Memory (KB)")
    plt.title("3D Convex Hull: Memory Usage")
    plt.legend()
    plt.grid(True, which="both", ls="--")

    plt.tight_layout()
    plt.show()
"""