# visualize_results_comprehensive.py
import os
import glob
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.animation import FuncAnimation

sns.set(style="whitegrid")

RESULTS_DIR = "../results"

# -----------------------------
# 1. Query summary CSV
# -----------------------------
summary_file = os.path.join(RESULTS_DIR, "query_summary.csv")
if os.path.exists(summary_file):
    df_summary = pd.read_csv(summary_file)
    df_summary['QueryType'] = df_summary['QueryType'].astype(str)

    # 1a. RTree vs Linear query time
    plt.figure(figsize=(10,6))
    for qt in df_summary['QueryType'].unique():
        subset = df_summary[df_summary['QueryType']==qt]
        plt.plot(subset.index, subset['RTreeTime(s)'], marker='o', label=f'{qt} RTree')
        plt.plot(subset.index, subset['LinearTime(s)'], marker='x', label=f'{qt} Linear')
    plt.xlabel("Query Index")
    plt.ylabel("Time (s)")
    plt.title("RTree vs Linear Query Time")
    plt.legend()
    plt.show()

    # 1b. Query time distribution (boxplot)
    plt.figure(figsize=(8,6))
    sns.boxplot(x='QueryType', y='RTreeTime(s)', data=df_summary)
    sns.boxplot(x='QueryType', y='LinearTime(s)', data=df_summary)
    plt.title("Query Time Distribution per Type")
    plt.show()

    # 1c. Speedup: Linear / RTree
    df_summary['Speedup'] = df_summary['LinearTime(s)'] / df_summary['RTreeTime(s)']
    plt.figure(figsize=(8,6))
    sns.barplot(x='QueryType', y='Speedup', data=df_summary)
    plt.title("RTree Speedup vs Linear Scan")
    plt.show()

    # 1d. Result counts per query type
    plt.figure(figsize=(8,6))
    df_counts = df_summary.groupby('QueryType')[['RTreeCount','LinearCount']].sum().reset_index()
    df_counts.plot(x='QueryType', kind='bar', stacked=False)
    plt.ylabel("Number of Results")
    plt.title("Result Counts per Query Type")
    plt.show()

    # 1e. kNN neighbors retrieved vs k
    knn_df = df_summary[df_summary['QueryType']=="kNearestNeighbors"]
    if not knn_df.empty:
        plt.figure(figsize=(8,6))
        plt.scatter(knn_df['k'], knn_df['RTreeCount'], label='RTree', marker='o')
        plt.scatter(knn_df['k'], knn_df['LinearCount'], label='Linear', marker='x')
        plt.xlabel("k")
        plt.ylabel("Number of neighbors retrieved")
        plt.title("kNN neighbors retrieved vs k")
        plt.legend()
        plt.show()

# -----------------------------
# 2. Distance CSVs (kNN and similarity)
# -----------------------------
distance_files = glob.glob(os.path.join(RESULTS_DIR, "query_*_rtree.csv"))

for file in distance_files:
    df_rtree = pd.read_csv(file)
    file_base = os.path.basename(file).replace(".csv","")

    if "kNN" in file_base:
        # kNN distance distribution
        plt.figure(figsize=(8,6))
        sns.histplot(df_rtree['Distance'], bins=20, kde=True, color='blue')
        plt.xlabel("Distance")
        plt.ylabel("Frequency")
        plt.title(f"kNN Distance Distribution ({file_base})")
        plt.show()

        # Trajectory length vs distance
        plt.figure(figsize=(8,6))
        plt.scatter(df_rtree['NumPoints'], df_rtree['Distance'], c=df_rtree['Distance'], cmap='viridis', alpha=0.7)
        plt.colorbar(label='Distance')
        plt.xlabel("Trajectory Length (#Points)")
        plt.ylabel("Distance")
        plt.title(f"Trajectory Length vs Distance (kNN) ({file_base})")
        plt.show()

        # Cumulative distribution of distances
        sorted_dist = np.sort(df_rtree['Distance'])
        cdf = np.arange(len(sorted_dist)) / len(sorted_dist)
        plt.figure(figsize=(8,6))
        plt.plot(sorted_dist, cdf)
        plt.xlabel("Distance")
        plt.ylabel("CDF")
        plt.title(f"Cumulative Distribution of kNN Distances ({file_base})")
        plt.show()

    if "findSimilar" in file_base:
        # Similarity score vs distance
        plt.figure(figsize=(8,6))
        plt.scatter(df_rtree['Similarity'], df_rtree['Distance'], alpha=0.7)
        plt.xlabel("Similarity Score")
        plt.ylabel("Distance")
        plt.title(f"Similarity Scores vs Distance ({file_base})")
        plt.show()

        # Trajectory length vs similarity
        plt.figure(figsize=(8,6))
        plt.scatter(df_rtree['NumPoints'], df_rtree['Similarity'], alpha=0.7)
        plt.xlabel("Trajectory Length (#Points)")
        plt.ylabel("Similarity Score")
        plt.title(f"Trajectory Length vs Similarity ({file_base})")
        plt.show()

        # Histogram of similarity scores
        plt.figure(figsize=(8,6))
        sns.histplot(df_rtree['Similarity'], bins=20, kde=True, color='green')
        plt.xlabel("Similarity Score")
        plt.ylabel("Frequency")
        plt.title(f"Similarity Score Distribution ({file_base})")
        plt.show()

# -----------------------------
# 3. Spatio-temporal plots
# -----------------------------
plot_files = glob.glob(os.path.join(RESULTS_DIR, "query_*_plot.csv"))

for file in plot_files:
    df_plot = pd.read_csv(file)
    file_base = os.path.basename(file).replace(".csv","")

    # 3a. 2D Trajectories
    plt.figure(figsize=(8,6))
    for ttype in df_plot['Type'].unique():
        subset = df_plot[df_plot['Type']==ttype]
        plt.plot(subset['X'], subset['Y'], label=ttype, alpha=0.7)
    plt.xlabel("X")
    plt.ylabel("Y")
    plt.title(f"Trajectories (2D) - {file_base}")
    plt.legend()
    plt.show()

    # 3b. 3D Spatio-temporal Trajectories
    fig = plt.figure(figsize=(10,7))
    ax = fig.add_subplot(111, projection='3d')
    for ttype in df_plot['Type'].unique():
        subset = df_plot[df_plot['Type']==ttype]
        ax.plot(subset['X'], subset['Y'], subset['T'], label=ttype, alpha=0.7)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("T")
    ax.set_title(f"Trajectories (3D) - {file_base}")
    ax.legend()
    plt.show()

    # 3c. Optional: density heatmap of trajectory points
    plt.figure(figsize=(8,6))
    sns.kdeplot(x=df_plot['X'], y=df_plot['Y'], fill=True, cmap='Reds', alpha=0.5)
    plt.xlabel("X")
    plt.ylabel("Y")
    plt.title(f"Trajectory Density Heatmap ({file_base})")
    plt.show()
