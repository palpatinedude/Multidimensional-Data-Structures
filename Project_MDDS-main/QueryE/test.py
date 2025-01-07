#!/usr/bin/env python
import matplotlib.pyplot as plt
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

class PSTNode:
    """Node in a Priority Search Tree."""
    def __init__(self, point):
        self.point = point  # The point stored in this node (x, y)
        self.left = None  # Left child
        self.right = None  # Right child

def build_pst(points):
    """Builds a Priority Search Tree from a list of points."""
    if not points:
        return None

    # Sort points by y-coordinate, then by x-coordinate
    points.sort(key=lambda point: (point[1], point[0]))

    # The median by x-coordinate to split the tree
    median_index = len(points) // 2
    median_point = points[median_index]

    root = PSTNode(median_point)
    root.left = build_pst(points[:median_index])
    root.right = build_pst(points[median_index+1:])

    return root

def query_pst(root, x1, x2, y):
    """Queries the PST for points within [x1, x2] and below y."""
    if root is None:
        return []

    results = []
    x, root_y = root.point

    # Check if the current node is within the query range
    if x1 <= x <= x2 and root_y <= y:
        results.append(root.point)

    # If the x-coordinate of the root is greater than or equal to x1, search left
    if x >= x1:
        results.extend(query_pst(root.left, x1, x2, y))
    # If the x-coordinate of the root is less than or equal to x2, search right
    if x <= x2:
        results.extend(query_pst(root.right, x1, x2, y))

    return results

def preprocess_points(points):
    """Preprocess to remove dominated points."""
    # Sort by x ascending and then by y descending
    points.sort(key=lambda p: (p[0], p[1]))
    filtered_points = []
    max_y = float('-inf')
    for x, y in points:
        if y > max_y:
            filtered_points.append((x, y))
            max_y = y
    return filtered_points

def is_dominated(point, other):
    """
    Check if 'point' is dominated by 'other'.
    A point is considered dominated if it is less than or equal to another point in both dimensions.
    """
    return point[0] >= other[0] and point[1] >= other[1]

def find_skyline(root):
    """
    Finds the skyline points from the PST, using dominance check to filter out dominated points.
    """
    if root is None:
        return []
    
    results = []
    left_skyline = find_skyline(root.left)
    for point in left_skyline:
        # Check if the point is dominated by the root's point
        if not is_dominated(point, root.point):
            results.append(point)

    # Append the root's point if it is not dominated by any point in the left skyline
    if not any(is_dominated(root.point, point) for point in left_skyline):
        results.append(root.point)
    
    right_skyline = find_skyline(root.right)
    for point in right_skyline:
        # Filter the right skyline to remove points dominated by the root or any point in the results so far
        if not any(is_dominated(point, other) for other in results):
            results.append(point)
    
    return results

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
        points.append((awrd,DBLP_Record_Hash))
        print("Points: ", points[i])
        print()
        i+=1
    
    # Convert the list to a NumPy array
    # points = np.array(points)
    # preprocessed_points = preprocess_points(points)
    preprocessed_points = points
    root = build_pst(preprocessed_points)
    skyline = find_skyline(root)
    print("Points:", points)
    print("Skyline:", skyline)

    # Plot the points and the skyline
    _, ax = plt.subplots()
    ax.scatter([p[0] for p in points], [p[1] for p in points], label='All Points')
    ax.scatter([p[0] for p in skyline], [p[1] for p in skyline], label='Skyline Points', color='red')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.legend()
    plt.show()


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(e)
        pass
