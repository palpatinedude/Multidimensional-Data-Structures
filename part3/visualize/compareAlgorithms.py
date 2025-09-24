"""
compareAlgorithms.py
-------------------
Compare performance (time & memory) of multiple convex hull algorithms (2D & 3D).

This script reads CSV result files from different algorithm folders, and visualizes:
1. Line plots (log-log) for time and memory comparison.
2. Bar charts for selected input sizes comparing algorithms.
3. It handles both single-run and multi-run CSV formats.
"""

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os

# -----------------------------
# CONFIG: list of algorithms to compare
# Each entry: (label, folder)
algorithms_2D = [
    ("Divide & Conquer", "../results/divideConquer2D/"),
    ("Monotone Chain", "../results/monotoneChain2D/"),
    ("QuickHull", "../results/quickHull2D/")
]

algorithms_3D = [
    ("Divide & Conquer", "../results/divideConquer3D/"),
    ("QuickHull", "../results/quickHull3D/")
]

# -----------------------------
# Helper: load CSV data from folder
def load_csv_data(folder):
    """
    Returns: n, time, memory, std_time, std_mem
    Handles both single-run and multi-run CSV formats.
    """
    csv_file = os.path.join(folder, os.path.basename(folder.rstrip('/')) + "_results.csv")
    if not os.path.exists(csv_file):
        print(f"CSV not found for {folder}")
        return None

    df = pd.read_csv(csv_file)
    n = df['n'].values

    # Detect multi-run or single-run format
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

    return n, time, mem, std_time, std_mem

# -----------------------------
# Line plot comparison for multiple algorithms
def plot_comparison(algorithms, title_prefix="2D"):
    plt.figure(figsize=(14,5))

    # Time comparison (log-log)
    plt.subplot(1,2,1)
    for label, folder in algorithms:
        data = load_csv_data(folder)
        if data:
            n, time, _, std_time, _ = data
            # Use error bars if standard deviation is available
            if std_time.max() > 0:
                plt.errorbar(n, time, yerr=std_time, fmt='o-', label=label)
            else:
                plt.plot(n, time, 'o-', label=label)
    plt.xlabel("Input size n")
    plt.ylabel("Time (us)")
    plt.title(f"{title_prefix} Convex Hull: Time Comparison")
    plt.xscale("log")
    plt.yscale("log")
    plt.grid(True, which="both", ls="--")
    plt.legend()

    # Memory comparison (log-log)
    plt.subplot(1,2,2)
    for label, folder in algorithms:
        data = load_csv_data(folder)
        if data:
            n, _, mem, _, std_mem = data
            if std_mem.max() > 0:
                plt.errorbar(n, mem, yerr=std_mem, fmt='o-', label=label)
            else:
                plt.plot(n, mem, 'o-', label=label)
    plt.xlabel("Input size n")
    plt.ylabel("Memory (KB)")
    plt.title(f"{title_prefix} Convex Hull: Memory Comparison")
    plt.xscale("log")
    plt.yscale("log")
    plt.grid(True, which="both", ls="--")
    plt.legend()

    plt.tight_layout()
    plt.show()

# -----------------------------
# Bar chart comparison for selected n values
def plot_bar_comparison(algorithms, title_prefix="2D", metric="time"):
    """
    Bar chart comparing algorithms for specific input sizes.
    metric: "time" or "memory"
    """
    # Choose input sizes to compare (adjust according to data)
    n_values = [100, 1000, 20000, 100000]

    alg_labels = [label for label, _ in algorithms]
    data_matrix = []

    # Gather metric values for each algorithm at selected n
    for label, folder in algorithms:
        data = load_csv_data(folder)
        if data:
            n, time, mem, _, _ = data
            values = []
            for val in n_values:
                idx = np.where(n == val)[0]
                if len(idx) > 0:
                    values.append(time[idx[0]] if metric=="time" else mem[idx[0]])
                else:
                    values.append(0)  # 0 if n not present
            data_matrix.append(values)

    # Plot bars side by side
    x = np.arange(len(n_values))
    width = 0.8 / len(algorithms)
    plt.figure(figsize=(10,5))
    for i, values in enumerate(data_matrix):
        plt.bar(x + i*width, values, width=width, label=alg_labels[i])

    plt.xticks(x + width*(len(algorithms)-1)/2, n_values)
    ylabel = "Time (us)" if metric=="time" else "Memory (KB)"
    plt.ylabel(ylabel)
    plt.xlabel("Input size n")
    plt.title(f"{title_prefix} Convex Hull: {ylabel} Comparison (Bar Chart)")
    plt.legend()
    plt.grid(True, axis="y", ls="--")
    plt.show()

# -----------------------------
# Main execution
if __name__ == "__main__":
    print("Plotting 2D algorithms comparison...")
    plot_comparison(algorithms_2D, title_prefix="2D")
    plot_bar_comparison(algorithms_2D, title_prefix="2D", metric="time")
    plot_bar_comparison(algorithms_2D, title_prefix="2D", metric="memory")

    print("Plotting 3D algorithms comparison...")
    plot_comparison(algorithms_3D, title_prefix="3D")
    plot_bar_comparison(algorithms_3D, title_prefix="3D", metric="time")
    plot_bar_comparison(algorithms_3D, title_prefix="3D", metric="memory")

