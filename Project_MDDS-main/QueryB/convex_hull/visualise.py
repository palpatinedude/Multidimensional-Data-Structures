#!/usr/bin/env python
import json
import matplotlib.pyplot as plt
import sys
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

def get_the_data_from_file(filename):
    with open(filename) as f:
        data = json.load(f)
    return data

def plot_points(data):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    # Assuming data['points'] is a list of dictionaries
    xs = [point['x'] for point in data['points']]  # x coordinates
    ys = [point['y'] for point in data['points']]  # y coordinates
    zs = [point['z'] for point in data['points']]  # z coordinates
    ax.scatter(xs, ys, zs)
    plt.show()

def plot_edges(data):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    for plane in data['planes']:
        edges = [plane['edge_a'], plane['edge_b'], plane['edge_c']]
        for edge in edges:
            p1, p2 = edge['start'], edge['end']
            x_values = [p1['x'], p2['x']]
            y_values = [p1['y'], p2['y']]
            z_values = [p1['z'], p2['z']]
            # Plot the edge
            ax.plot(x_values, y_values, z_values, 'k-')
    plt.show()


def plot_planes(data):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Assuming data['points'] is a list of dictionaries
    xs = [point['x'] for point in data['points']]  # x coordinates
    ys = [point['y'] for point in data['points']]  # y coordinates
    zs = [point['z'] for point in data['points']]  # z coordinates
    ax.scatter(xs, ys, zs)

    for plane in data['planes']:
        # Extract the coordinates for each point
        xa, ya, za = plane['point_a']['x'], plane['point_a']['y'], plane['point_a']['z']
        xb, yb, zb = plane['point_b']['x'], plane['point_b']['y'], plane['point_b']['z']
        xc, yc, zc = plane['point_c']['x'], plane['point_c']['y'], plane['point_c']['z']
        
        # Create vertices list
        vertices = [[xa, ya, za], [xb, yb, zb], [xc, yc, zc]]
        
        # Plot each plane
        tri = Poly3DCollection([vertices], alpha=0.5, edgecolors='k')
        ax.add_collection3d(tri)
        
        # Optionally, plot the normal vector
        # Calculate the centroid of the triangle to position the normal
        centroid_x = (xa + xb + xc) / 3
        centroid_y = (ya + yb + yc) / 3
        centroid_z = (za + zb + zc) / 3
        normal = plane['normal']

        ax.quiver(centroid_x, centroid_y, centroid_z, normal['x'], normal['y'], normal['z'], length=0.5, normalize=True)
    plt.show()

def main():
    if len(sys.argv) != 2:
        print("Usage: ./visualiser.py [points|edges|planes]")
        sys.exit(1)

    mode = sys.argv[1]
    data = get_the_data_from_file('convex_hull.json')

    if mode == 'points':
        plot_points(data)
    elif mode == 'edges':
        plot_edges(data)
    elif mode == 'planes':
        plot_planes(data)
    else:
        print("Invalid argument. Use 'points', 'edges', or 'planes'.")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting...")
        sys.exit(0)
    except Exception as e:
        print("An error occurred: {}".format(e))
        sys.exit(1)
