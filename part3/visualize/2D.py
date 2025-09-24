"""
2D Convex Hull Visualization and Performance Analysis

This script loads 2D point sets and their convex hulls computed
by different algorithms (Divide & Conquer, Monotone Chain, QuickHull)
from the results folder, visualizes them, and plots
time and memory performance from CSV results.

- Understand how input size 'n' is encoded in filenames.
"""

import matplotlib.pyplot as plt
import numpy as np
import glob
import pandas as pd
import os
import re

# -----------------------------
# CONFIG: set folder containing results for one algorithm
#folder = "../results/divideConquer2D/"
#folder = "../results/monotoneChain2D/"
folder = "../results/grahamScan2D/"
#folder = "../results/quickHull2D/"
# -----------------------------

# -----------------------------
# Helper: load 2D points from a text file
def load_points_2D(file_path):
    points = []
    with open(file_path) as f:
        for line in f:
            x, y = map(float, line.split())
            points.append((x, y))
    return np.array(points)

# Helper: extract input size 'n' from filename
def extract_n(filename):
    """
    Extracts 'n' from filenames like:
      divideConquer2D_points_100_1.txt
      divideConquer2D_hull_100_1.txt
    """
    base = os.path.basename(filename)
    match = re.search(r'_(?:points|hull)_(\d+)', base)
    if match:
        return int(match.group(1))
    else:
        raise ValueError(f"Cannot extract n from filename: {filename}")

# -----------------------------
# Load points and hull files
points_files = sorted(glob.glob(os.path.join(folder, "*_points_*.txt")))
hull_files   = sorted(glob.glob(os.path.join(folder, "*_hull_*.txt")))

# Only consider the first run per input size
unique_n = sorted(set(extract_n(f) for f in points_files))
for n in unique_n:
    pts_file  = next(f for f in points_files if extract_n(f) == n)
    hull_file = next(f for f in hull_files   if extract_n(f) == n)

    points = load_points_2D(pts_file)
    hull   = load_points_2D(hull_file)

    # -----------------------------
    # Plot points and hull
    plt.figure(figsize=(6,6))
    plt.scatter(points[:,0], points[:,1], color='blue', label='Points')

    # Draw hull edges
    for i in range(len(hull)):
        x = [hull[i,0], hull[(i+1)%len(hull),0]]
        y = [hull[i,1], hull[(i+1)%len(hull),1]]
        plt.plot(x, y, 'r-')

    plt.title(f"2D Convex Hull n={n}")
    plt.xlabel("X")
    plt.ylabel("Y")
    plt.legend()
    plt.grid(True)
    plt.show()

# -----------------------------
# Load CSV performance results and plot
csv_file = os.path.join(folder, os.path.basename(folder.rstrip('/')) + "_results.csv")
if os.path.exists(csv_file):
    df = pd.read_csv(csv_file)
    print("CSV loaded:", csv_file)
    print(df.head())

    # Determine if CSV contains averages or single-run results
    if 'avg_time_us' in df.columns:
        time_col = df['avg_time_us'].values
        mem_col  = df['avg_mem_KB'].values
        std_time = df['std_time_us'].values
        std_mem  = df['std_mem_KB'].values
    else:
        time_col = df['time_us'].values
        mem_col  = df['approx_mem_KB'].values
        std_time = np.zeros_like(time_col)
        std_mem  = np.zeros_like(mem_col)

    n_vals = df['n'].values

    plt.figure(figsize=(12,5))

    # ----------------- Time plot -----------------
    plt.subplot(1,2,1)
    if std_time.max() > 0:
        plt.errorbar(n_vals, time_col, yerr=std_time, fmt='bo-', label='Measured time')
    else:
        plt.plot(n_vals, time_col, 'bo-', label='Measured time')

    # Reference O(n log n) curve for comparison
    plt.plot(n_vals, time_col[0]*(n_vals*np.log2(n_vals)/(n_vals[0]*np.log2(n_vals[0]))), 'r--', label='O(n log n) reference')
    plt.xlabel("Input size n")
    plt.ylabel("Time (us)")
    plt.title("2D Convex Hull: Time Complexity")
    plt.legend()
    plt.grid(True, which="both", ls="--")

    # ----------------- Memory plot -----------------
    plt.subplot(1,2,2)
    if std_mem.max() > 0:
        plt.errorbar(n_vals, mem_col, yerr=std_mem, fmt='go-', label='Measured memory')
    else:
        plt.plot(n_vals, mem_col, 'go-', label='Measured memory')

    # Reference O(n) memory usage
    plt.plot(n_vals, mem_col[0]*(n_vals/n_vals[0]), 'r--', label='O(n) reference')
    plt.xlabel("Input size n")
    plt.ylabel("Memory (KB)")
    plt.title("2D Convex Hull: Memory Usage")
    plt.legend()
    plt.grid(True, which="both", ls="--")

    plt.tight_layout()
    plt.show()
