#!/usr/bin/env python
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
        print('\033[91mQuery:', query[0], end=' ')
        print('Result:\033[0m', end=' ')
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
        print('\033[91mQuery:', query, end=' ')
        print('Result:\033[0m', end=' ')
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
