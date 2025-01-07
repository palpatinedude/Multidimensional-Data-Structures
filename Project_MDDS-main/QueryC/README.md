# Skyline Operator & Convex Hull

## Overview
Το παρακάτο υποερωτημα του project αποτελεί την υλοποίηση του αλγορίθμου του `2D Convex Hull` και του `2D Skyline Operator` σε Python. 

Πιο συγκεκριμένα, ο αλγόριθμος που υλοποιήθηκε είναι ο αλγόριθμος του `Graham Scan`. Ο αλγόριθμος αυτός είναι ένας αλγόριθμος που χρησιμοποιείται για την εύρεση του `Convex Hull` ενός συνόλου σημείων στον δυσδιάστατο χώρο (στην περίπτωση μας). 

Ο αλγόριθμος `Skyline Operator` 2D επιστρέφει το σύνολο των σημείων που αποτελούν το Skyline, δηλαδή τα σημεία που δεν μπορούν να κυριαρχηθούν από άλλα σημεία σε κάποιο άλλο κριτήριο. Στην περίπτωση μας υπολογίζουμε τα 4 υποσύνολα: `1o Subset` μικρότερες τιμές για d1 και d2, `2o Subset` μια μικρότερη τιμή d1 και μια μεγαλύτερη τιμή d2, `3o Subset` μια μεγαλύτερη τιμή d1 και μια μικρότερη τιμή d2, `4o Subset` μεγαλύτερες τιμές σε όλες τις διαστάσεις.

## Table of Contents
- [Skyline Operator & Convex hull](#skyline-operator--convex-hull)
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Installation & Requirements](#installation--requirements)
  - [Usage](#usage)
  - [Code Documentation](#code-documentation)
    - [Functions](#functions)
    - [Code](#code)

## Table of Contents

## Installation & Requirements
Τα requirements για την εκτελσεση του κώδικα είναι τα εξής:
- Εγκατάσταση της Python
  - Για την εγκατάσταση της Python, ακολουθήστε τις οδηγίες που βρίσκονται στον παρακάτω σύνδεσμο: [Python Installation](https://www.python.org/downloads/)
  - Εγκατάσταση του pip
    - Μετά την εγκατάσταση της Python, εγκαταστήστε το pip ακολουθώντας τις οδηγίες που βρίσκονται στον παρακάτω σύνδεσμο: [Pip Installation](https://pip.pypa.io/en/stable/installation/)
  - Εγκατάσταση του lib `matplotlib`
    - Μετά την εγκατάσταση του pip, εγκαταστήστε το lib `matplotlib` μέσω του pip με την εντολή: `pip install matplotlib`
  -  Εγκατάσταση του lib `NumPy`
    - Μετά την εγκατάσταση του pip, εγκαταστήστε το lib `NumPy` μέσω του pip με την εντολή: `pip install numpy`

## Usage

Για την εκτέλεση του κώδικα ακολουθήστε τα παρακάτω βήματα:
- Για το αρχείο test.py τρεξτε τον κώδικα με την εντολή `./test.py`

## Code Documentation

### Functions
- `def convex_hull(points):`: Περιέχει τον κώδικα του αλγορίθμου του `Graham Sca`n καθως και την υλοποίηση του `Convex Hull` structure ενος συνόλου 2D σημείων.
- `def orientation(p, q, r):`: Συγκρίνει 3 σημεία και βρίσκει τον προσανατολισμό τους. Επιστρέφει 0 αν είναι κολινέα, 1 αν είναι δεξιόστροφος και 2 αν είναι αριστερόστροφος.
  - Βοηθητική συνάρτηση για την εύρεση του `Convex Hull`.
- `def skyline`: Περιέχει τον κώδικα για την υλοποίηση των skyline σημείων στον δυσδιάστατο χώρο. 
- `def is_dominated`: Είναι βοηθιτική συνάρτηση για την υλοποίηση του `Skyline Operator`. Περιέχει τον κώδικα για την εύρεση των σημείων που δεν κυριαρχούνται από όλα τα υπόλοιοπα. Ανάλογα με το case που επιλέγουμε βρίσκει τα σημεία για:
  - `1o subset:` MIN d1, MIN d2
  - `2o subset:` MIN d1, MAX d2
  - `3o subset`: MAX d1, MIN d2
  - `4o subset`: MAX d1, MAX d2  
- `hash_function(string):`: Περιέχει τον κώδικα για την μετατροπή του DBLP_Records το οποίο είναι σε μορφή string σε μορφή integer μεσω του hashing.


### Code

```python
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
```
