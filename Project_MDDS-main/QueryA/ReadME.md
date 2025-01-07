
## Overview
Στο πιο κατω υποερωτημα ζητητε τη δημιουργία δομών δεδομένων για την αποθήκευση και την αναζήτηση χρονικών διαστημάτων 
δημοσιεύσεων επιστημόνων. Συγκεκριμενα ζητητε η δημιουργια ενως Interval tree και η δημιουργια ενως Segment tree.
Για την υλοποίηση των δομών δεδομένων αυτών χρησιμοποιήθηκε η γλώσσα προγραμματισμού Python. Στο παρακάτω κείμενο θα παρουσιαστεί ο κώδικας για την δημιουργία των δομών δεδομένων αυτών και ο τρόπος εκτέλεσης τους.

## Table of contents
- [Installation & Requirements](#installation-&-requirements)
- [Usage](#usage)
- [Basic Usage](#basic-usage)
- [code Documentation](#code-documentation)
- [Modules](#modules)
- [Fuctions](#fuctions)
    - [Interval Tree](#interval-tree)
    - [Segment Tree](#segment-tree)



## Installation & Requirements
Τα requirements για την εκτελσεση του κώδικα είναι τα εξής:
1.  - Για την εγκατάσταση της Python, ακολουθήστε τις οδηγίες που βρίσκονται στον παρακάτω σύνδεσμο: 
       [Python Installation](https://www.python.org/downloads/)
    - Εγκατάσταση του pip
    - Μετά την εγκατάσταση της Python, εγκαταστήστε το pip ακολουθώντας τις οδηγίες που βρίσκονται στον παρακάτω σύνδεσμο: 
      [Pip Installation](https://pip.pypa.io/en/stable/installation/)




## Usage
## Basic Usage
Για την εκτέλεση του κώδικα, ακολουθήστε τα παρακάτω βήματα:
 -Για το Interval Tree
    - Ανοίξτε το αρχείο interval_tree.py
    - Εκτελέστε τον κώδικα
 -Για το Segment Tree
    - Ανοίξτε το αρχείο segment_tree.py
    - Εκτελέστε τον κώδικα
    


## code Documentation
## Modules
- `test.py` : Περιέχει τον κώδικα για την δημιουργία του Interval Tree , στο φακελο intervalTree
- `test.py` : Περιέχει τον κώδικα για την δημιουργία του Segment Tree , στο φακελο segmentTree
## Fuctions 


 ## Interval Tree 
 Εδω αναλυουμε τον κώδικα για την δημιουργία του Interval Tree και τις συναρτησεις του :
 -κλαση Node : Περιέχει τα δεδομένα του κάθε κόμβου του δέντρου
 -κλαση IntervalTree : Περιέχει τις συναρτησεις για την δημιουργία του δέντρου και τις συναρτησεις για την αναζήτηση των δεδομένων
 -Συναρτηση main : Περιέχει τον κώδικα για την εκτέλεση του προγράμματος


  ```python
  import json

class Node:
    def __init__(self, data):
        self.data = data
        self.start = data['gap of years'][0]
        self.end = data['gap of years'][1]
        self.max_end = self.end
        self.left = None
        self.right = None

class IntervalTree:
    def __init__(self, records):
        self.root = None
        for record in records:
            self.insert(record)

    def insert(self, record, node=None):
        start, end = record['gap of years']
        if not node:
            node = self.root
        if not node:
            self.root = Node(record)
            return
        if start < node.start:
            if not node.left:
                node.left = Node(record)
            else:
                self.insert(record, node.left)
        else:
            if not node.right:
                node.right = Node(record)
            else:
                self.insert(record, node.right)
        node.max_end = max(node.max_end, end)

    def interval_query(self, start, end, node=None, results=None):
        if results is None:
            results = []
        if not node:
            node = self.root

        # Stop if we reach a leaf node
        if not node:
            return results

        # If the current node's interval intersects with the query interval, add it to results
        if start <= node.end and end >= node.start:
            results.append(node.data)

        # Traverse the left subtree if its intervals might intersect with the query interval
        if node.left and start <= node.left.max_end:
            self.interval_query(start, end, node.left, results)

        # Traverse the right subtree if its intervals might intersect with the query interval
        if node.right and end >= node.start:
            self.interval_query(start, end, node.right, results)

        return results

    def query(self, point, node=None, results=None):
        if results is None:
            results = []
        if not node:
            node = self.root
        if node:
            if point >= node.start and point <= node.end:
                # results.append((node.start, node.end))
                results.append(node.data)
            if node.left and point <= node.left.max_end:
                self.query(point, node.left, results)
            if node.right:
                self.query(point, node.right, results)
        return results

def main():
    filename = '../../pol.json'
    data= json.load(open(filename, 'r'))

    # Create Interval Tree
    interval_tree = IntervalTree(data)

    queries = [
        (1995, 1997),
        (1998, 2000),
        (2004, 2005),
        (2009, 2012),
        (2016, 2017),
        (2021, 2023),
    ]


    # Stabbing querying
    print('Stabbing querying:')
    for query in queries:
        print("Query:", query[0], end=" ")
        print("Result:", end=" ")
        authors = []
        for i in interval_tree.query(query[0]):
            #print(i['gap of years'], end=" ")
            if i['author\'s name'] not in authors:
                authors.append(i['author\'s name'])
                print(i['author\'s name'], end=" ")
        print()

    print()

    # Interval querying
    print('Interval querying:')
    for query in queries:
        print("Query:", query, end=" ")
        print("Result:", end=" ")
        authors = []
        for i in interval_tree.interval_query(query[0], query[1]):
            #print(i['gap of years'], end=" ")
            if i['author\'s name'] not in authors:
                authors.append(i['author\'s name'])
                print(i['author\'s name'], end=" ")
        print()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(e)
        pass
```


## Segment Tree
Εδω αναλυουμε τον κώδικα για την δημιουργία του Segment Tree και τις συναρτησεις του :
-κλαση SegmentTreeNode : Περιέχει τα δεδομένα του κάθε κόμβου του δέντρου
-κλαση SegmentTree : Περιέχει τις συναρτησεις για την δημιουργία του δέντρου και τις συναρτησεις για την αναζήτηση των δεδομένων(διαχειρηση του δεντρου)
-Συναρτηση main : Περιέχει τον κώδικα για την εκτέλεση του προγράμματος


```python
import json

class SegmentTreeNode:
    def __init__(self, start, end):
        self.start = start
        self.end = end
        self.intervals = []
        self.left = None
        self.right = None

class SegmentTree:
    def __init__(self, intervals):
        # Determine the bounds of the tree
        all_points = [interval['gap of years'][0] for interval in intervals] + \
                     [interval['gap of years'][1] for interval in intervals]
        self.root = self.build_tree(min(all_points), max(all_points))
        for interval in intervals:
            self.insert(interval)

    def build_tree(self, start, end):
        if start > end:
            return None
        node = SegmentTreeNode(start, end)
        if start != end:
            mid = (start + end) // 2
            node.left = self.build_tree(start, mid)
            node.right = self.build_tree(mid + 1, end)
        return node

    def insert(self, interval):
        self._insert_node(self.root, interval)

    def _insert_node(self, node, interval):
        if not node:
            return
        start, end = interval['gap of years']
        if end < node.start or start > node.end:
            return
        if start <= node.start and end >= node.end:
            node.intervals.append(interval)
            return
        self._insert_node(node.left, interval)
        self._insert_node(node.right, interval)

    def query(self, point):
        return self._query_node(self.root, point, True)

    def interval_query(self, query_start, query_end):
        return self._query_node(self.root, query_start, False, query_end)

    def _query_node(self, node, point, is_query, end=None):
        if not node:
            return []
        if is_query:
            if point < node.start or point > node.end:
                return []
        else:
            if end < node.start or point > node.end:
                return []

        results = []
        added_intervals = set()  # Track unique intervals by their JSON representation

        def add_interval(interval):
            interval_json = json.dumps(interval, sort_keys=True)  # Convert interval to JSON string for uniqueness
            if interval_json not in added_intervals:
                results.append(interval)
                added_intervals.add(interval_json)

        for interval in node.intervals:
            if (is_query and interval['gap of years'][0] <= point <= interval['gap of years'][1]) or \
               (not is_query and interval['gap of years'][0] <= end and interval['gap of years'][1] >= point):
                add_interval(interval)

        for child_node in [node.left, node.right]:
            child_results = self._query_node(child_node, point, is_query, end)
            for interval in child_results:
                add_interval(interval)

        return results
    
def main():
    # Read data from JSON file
    filename = '../../pol.json'
    data = json.load(open(filename, 'r'))

    # Create Segment Tree
    st = SegmentTree(data)

    queries = [
        (1995, 1997),
        (1998, 2000),
        (2004, 2005),
        (2009, 2012),
        (2016, 2017),
        (2021, 2023),
    ]

    # Stabbing querying
    print('Stabbing querying:')
    for query in queries:
        print('Query:', query[0], end=' ')
        print('Result:', end=' ')
        authors = []
        for i in st.query(query[0]):
            #print(i['gap of years'], end=' ')
            if i['author\'s name'] not in authors:
                authors.append(i['author\'s name'])
                print(i['author\'s name'], end=" ")
        print()

    print()

    # Interval querying
    print('Interval querying:')
    for query in queries:
        print('Query:', query, end=' ')
        print('Result:', end=' ')
        authors = []
        for i in st.interval_query(*query):
            #print(i['gap of years'], end=' ')
            if i['author\'s name'] not in authors:
                authors.append(i['author\'s name'])
                print(i['author\'s name'], end=" ")
        print()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(e)
        pass

```


    



