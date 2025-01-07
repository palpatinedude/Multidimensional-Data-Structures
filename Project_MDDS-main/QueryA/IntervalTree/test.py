#!/usr/bin/env python
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
