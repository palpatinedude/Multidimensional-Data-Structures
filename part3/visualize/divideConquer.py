import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from scipy.spatial import ConvexHull
import numpy as np

# ------------------------------
# Load 2D points
# ------------------------------
points2D = []
with open("../results/divideConquer/points2D.txt") as f:
    for line in f:
        x, y = map(float, line.split())
        points2D.append((x, y))

# Load 3D points
points3D = []
with open("../results/divideConquer/points3D.txt") as f:
    for line in f:
        x, y, z = map(float, line.split())
        points3D.append((x, y, z))

# Convert to numpy arrays
points2D = np.array(points2D)
points3D = np.array(points3D)

# ------------------------------
# Compute 2D hull using scipy (for plotting filled polygon)
# ------------------------------
hull2D = ConvexHull(points2D)

plt.figure(figsize=(6, 6))
plt.scatter(points2D[:, 0], points2D[:, 1], color='blue', label='Points')

# Draw hull polygon
for simplex in hull2D.simplices:
    plt.plot(points2D[simplex, 0], points2D[simplex, 1], 'r-')

plt.title("2D Convex Hull")
plt.xlabel("X")
plt.ylabel("Y")
plt.legend()
plt.grid(True)
plt.show()

# ------------------------------
# Compute 3D hull using scipy
# ------------------------------
hull3D = ConvexHull(points3D)

fig = plt.figure(figsize=(8, 6))
ax = fig.add_subplot(111, projection='3d')

# Plot 3D points
ax.scatter(points3D[:, 0], points3D[:, 1], points3D[:, 2], color='blue', label='Points')

# Add hull faces as filled polygons
faces = [points3D[simplex] for simplex in hull3D.simplices]
poly3d = Poly3DCollection(faces, alpha=0.3, facecolor='red', edgecolor='black')
ax.add_collection3d(poly3d)

ax.set_title("3D Convex Hull")
ax.set_xlabel("X")
ax.set_ylabel("Y")
ax.set_zlabel("Z")
plt.legend()
plt.show()
