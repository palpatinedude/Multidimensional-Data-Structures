#!/usr/bin/env python
import matplotlib.pyplot as plt
import numpy as np
import json


with open('../.env', 'r') as file:
    # Read the environment file line by line
    for line in file:
        # Split the line by '='
        key, value = line.split('=')
        # Remove newline character from value
        value = value.strip()
        # Set the environment variable
        globals()[key] = int(value)

def hash_function(string):
    """Simple hash function to convert a string to a number."""
    return sum([ord(c) for c in string]) % DBLP_RECORDS_LENGTH

def orientation(p, q, r):
    """Calculate orientation of ordered triplet (p, q, r). 
    Returns 0 if collinear, 1 if clockwise, 2 if counterclockwise."""
    val = (q[1] - p[1]) * (r[0] - q[0]) - (q[0] - p[0]) * (r[1] - q[1])
    if val == 0: return 0  # Collinear
    return 1 if val > 0 else 2  # Clock or counterclockwise

def convex_hull(points):
    """Perform Graham Scan to find the convex hull of a set of 2D points."""
    n = len(points)
    if n < 3: return  # Convex hull not possible with less than 3 points

    # Find the bottom-most point (or choose the left most point in case of tie)
    points = sorted(points, key=lambda p: (p[1], p[0]))

    # Sort the remaining points based on their angle with the first point
    sorted_pts = sorted(points[1:], key=lambda p: np.arctan2(p[1] - points[0][1], p[0] - points[0][0]))

    # Place the bottom-most point back in the sorted list
    sorted_pts.insert(0, points[0])

    # Create an empty stack and push first three points
    hull = sorted_pts[:3]

    # Process remaining points
    for p in sorted_pts[3:]:
        while len(hull) > 1 and orientation(hull[-2], hull[-1], p) != 2:
            hull.pop()
        hull.append(p)

    return np.array(hull)

def is_dominated(point, others, dominance_case):
    for other in others:
        if not (point[0] == other[0] and point[1] == other[1]):  # Compare elements individually
            # Case 1: MIN d1, MIN d2 (lower-left dominance)
            if dominance_case == 1 and other[0] <= point[0] and other[1] <= point[1]:
                return True
            # Case 2: MIN d1, MAX d2 (upper-left dominance)
            elif dominance_case == 2 and other[0] <= point[0] and other[1] >= point[1]:
                return True
            # Case 3: MAX d1, MIN d2 (lower-right dominance)
            elif dominance_case == 3 and other[0] >= point[0] and other[1] <= point[1]:
                return True
            # Case 4: MAX d1, MAX d2 (upper-right dominance)
            elif dominance_case == 4 and other[0] >= point[0] and other[1] >= point[1]:
                return True
    return False

def skyline(points, dominance_case=1):
    """Find all points in the skyline."""
    skyline_points = []
    for point in points:
        # if not is_dominated(point, points):
        if not is_dominated(point, points, dominance_case):
            skyline_points.append(point)
    return np.array(skyline_points)

def main():
    # Path to your JSON file
    filename = '../pol.json'
    data= json.load(open(filename, 'r'))
    
    points=[]
    # Iterate through each record
    i=0
    for record in data:
        # Get year of release of each record
        if 'Awards' in record:
            awrd = record['Awards']
            print("Awards:", awrd)
        # Convert DBLP_Record (str) to a hash
        if 'DBLP_Record' in record:
            DBLP_Record_Hash = hash_function(record['DBLP_Record'])
            print(" DBLP_Record:", record['DBLP_Record'], "  DBLP_Record_Hash:", DBLP_Record_Hash)
        points.append((awrd, DBLP_Record_Hash))
        print("Points: ", points[i])
        print()
        i+=1
    
    # Convert the list to a NumPy array
    points = np.array(points)
    hull_points = convex_hull(points)
    skyline_points = skyline(points, dominance_case=1)
    sorted_skyline_points = skyline_points[np.argsort(skyline_points[:, 0])]

    # First Plot - Points and Convex Hull
    plt.figure()
    plt.scatter(points[:,0], points[:,1], label='Points')
    for i in range(len(hull_points)):
        plt.plot([hull_points[i][0], hull_points[(i+1) % len(hull_points)][0]], 
                 [hull_points[i][1], hull_points[(i+1) % len(hull_points)][1]], 'r')
    plt.legend()

    # Second Plot - All Points with Skyline Highlighted
    plt.figure()
    plt.scatter(points[:,0], points[:,1], label='Points')
    plt.scatter(skyline_points[:,0], skyline_points[:,1], color='green', label='Skyline Points')
    for i in range(len(sorted_skyline_points) - 1):
        plt.plot([sorted_skyline_points[i][0], sorted_skyline_points[i+1][0]], 
                 [sorted_skyline_points[i][1], sorted_skyline_points[i+1][1]], 'g--')
    plt.legend()
    plt.show()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(e)
        pass
