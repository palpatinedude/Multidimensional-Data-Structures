// test_RTreeNode.cpp

#include "../api/include/RTreeNode.h"
#include "../api/include/point3D.h"
#include "../api/include/bbox3D.h"
#include "../api/include/trajectory.h"

#include <iostream>
#include <cassert>
#include <cmath>

using namespace std;

// ----------- Tests for Point3D --------------

// Adjusted to handle Point3D with string timestamp 't'
void testPoint3DCreationAndEquality() {
    Point3D p1(1.0f, 2.0f, "2023-01-01T10:00:00Z");
    Point3D p2(1.0f, 2.0f, "2023-01-01T10:00:00Z");
    Point3D p3(3.0f, 2.0f, "2023-01-01T10:05:00Z");

    assert(p1 == p2);
    assert(!(p1 == p3));
    assert(p1 != p3);

    cout << "[PASS] Point3D creation and equality (with string timestamp)\n";
}

void testPoint3DDistance() {
    Point3D p1(0, 0, "2023-01-01T00:00:00Z");
    Point3D p2(1, 1, "2023-01-01T00:00:05Z");  // timestamp irrelevant for distance
    float dist = p1.distanceTo(p2);

    assert(abs(dist - sqrt(2)) < 1e-5);

    cout << "[PASS] Point3D distance calculation\n";
}

// ----------- Tests for BoundingBox3D --------------

void testBoundingBoxCreationAndExpansion() {
    BoundingBox3D box;
    Point3D p1(1, 2, "2023-01-01T10:00:00Z");
    box.expandToInclude(p1);

    Point3D p2(4, 5, "2023-01-01T10:05:00Z");
    box.expandToInclude(p2);

    assert(box.getMinX() <= 1 && box.getMinY() <= 2);
    assert(box.getMaxX() >= 4 && box.getMaxY() >= 5);

    cout << "[PASS] BoundingBox3D creation and expansion\n";
}

void testBoundingBoxVolumeAndIntersection() {
    BoundingBox3D box1;
    box1.expandToInclude(Point3D(0, 0, "2023-01-01T00:00:00Z"));
    box1.expandToInclude(Point3D(3, 3, "2023-01-01T00:03:00Z"));

    BoundingBox3D box2;
    box2.expandToInclude(Point3D(1, 1, "2023-01-01T00:00:00Z"));
    box2.expandToInclude(Point3D(4, 4, "2023-01-01T00:04:00Z"));

    float vol1 = box1.volume();
    float vol2 = box2.volume();
    assert(vol1 > 0);
    assert(vol2 > 0);

    bool intersect = box1.intersects(box2);
    assert(intersect);

    BoundingBox3D box3;
    box3.expandToInclude(Point3D(5, 5, "2024-01-01T00:05:00Z"));
    box3.expandToInclude(Point3D(6, 6, "2024-01-01T00:06:00Z"));
    assert(!box1.intersects(box3));

    cout << "[PASS] BoundingBox3D volume and intersection\n";
}

// ----------- Tests for Trajectory --------------

Trajectory createTrajectory(const std::vector<Point3D>& pts, const std::string& id) {
    return Trajectory(pts, id);
}

void testTrajectoryCreationAndMBR() {
    vector<Point3D> pts = {Point3D(1, 2, "2023-01-01T10:00:00Z"), Point3D(4, 5, "2023-01-01T10:05:00Z")};
    Trajectory traj(pts, "test_id");

    assert(traj.getId() == "test_id");

    BoundingBox3D mbr = traj.computeBoundingBox();
    assert(mbr.getMinX() == 1 && mbr.getMinY() == 2);
    assert(mbr.getMaxX() == 4 && mbr.getMaxY() == 5);

    cout << "[PASS] Trajectory creation and MBR computation\n";
}

void testTrajectoryDistanceAndEquality() {
    Trajectory t1 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");
    Trajectory t2 = createTrajectory({Point3D(1, 1, "2023-01-01T00:01:00Z")}, "t2");
    Trajectory t3 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");

    assert(t1 != t2);
    assert(t1 == t3);

    float dist = t1.distanceTo(t2);
    assert(dist > 0);

    cout << "[PASS] Trajectory distance and equality\n";
}

// ----------- Tests for RTreeNode (existing + new) --------------

void testConstructorAndIsLeaf() {
    auto leafNode = std::make_shared<RTreeNode>(true, 4);
    assert(leafNode->isLeafNode() == true);

    auto internalNode = std::make_shared<RTreeNode>(false, 4);
    assert(internalNode->isLeafNode() == false);

    cout << "[PASS] Constructor and isLeafNode()\n";
}

void testInsertAndNeedsSplit() {
    // Root is an internal node with max 2 entries
    auto root = std::make_shared<RTreeNode>(false, 2);

    // Create trajectories
    Trajectory t1 = createTrajectory({Point3D(0,0,"2023-01-01T00:00:00Z")}, "t1");
    Trajectory t2 = createTrajectory({Point3D(1,1,"2023-01-01T00:01:00Z")}, "t2");
    Trajectory t3 = createTrajectory({Point3D(2,2,"2023-01-01T00:02:00Z")}, "t3");

    // Wrap trajectories in leaf nodes
    auto leaf1 = std::make_shared<RTreeNode>(true, 2);
    auto leaf2 = std::make_shared<RTreeNode>(true, 2);
    auto leaf3 = std::make_shared<RTreeNode>(true, 2);

    // Insert trajectories into leaf nodes
    leaf1->insertLeaf(t1.computeBoundingBox(), std::make_shared<Trajectory>(t1));
    leaf2->insertLeaf(t2.computeBoundingBox(), std::make_shared<Trajectory>(t2));
    leaf3->insertLeaf(t3.computeBoundingBox(), std::make_shared<Trajectory>(t3));

    // Insert leaf nodes into root using getBoundingBox()
    root->insertChild(leaf1->getMBR(), leaf1);
    root->insertChild(leaf2->getMBR(), leaf2);

    // Root should not need split yet
    assert(root->needsSplit() == false);

    // Insert third leaf node: may trigger split depending on max entries
    root->insertChild(leaf3->getMBR(), leaf3);

    // Check if root needs split
    if (root->needsSplit()) {
        std::cout << "Root node needs split as expected.\n";
    } else {
        std::cout << "Root node did not need split yet.\n";
    }

    std::cout << "testInsertAndNeedsSplit finished.\n";
}


void testChooseSubtree() {
    auto root = std::make_shared<RTreeNode>(false, 3);

    // Insert 3 child nodes with different MBRs
    for (int i = 0; i < 3; ++i) {
        auto child = std::make_shared<RTreeNode>(true, 3);
        BoundingBox3D box;
        box.expandToInclude(Point3D(i * 10, i * 10, "2023-01-01T10:0" + to_string(i) + ":00Z"));
        box.expandToInclude(Point3D(i * 10 + 5, i * 10 + 5, "2023-01-01T10:0" + to_string(i) + ":05Z"));
        root->insertChild(box, child);
    }

    BoundingBox3D queryBox;
    queryBox.expandToInclude(Point3D(12, 12, "2023-01-01T10:12:00Z"));
    queryBox.expandToInclude(Point3D(14, 14, "2023-01-01T10:14:00Z"));

    int chosenIndex = root->chooseSubtree(queryBox);
    assert(chosenIndex >= 0 && chosenIndex < 3);

    cout << "[PASS] chooseSubtree()\n";
}

void testRangeQuery() {
    auto root = std::make_shared<RTreeNode>(true, 3);

    Trajectory t1 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");
    Trajectory t2 = createTrajectory({Point3D(10, 10, "2023-01-01T00:10:00Z")}, "t2");
    root->insertRecursive(t1);
    root->insertRecursive(t2);

    BoundingBox3D queryBox;
    queryBox.expandToInclude(Point3D(-1, -1, "2023-01-01T00:00:00Z"));
    queryBox.expandToInclude(Point3D(1, 1, "2023-01-01T00:01:00Z"));

    vector<Trajectory> results;
    root->rangeQuery(queryBox, results);

    assert(results.size() == 1);
    assert(results[0].getId() == "t1");

    cout << "[PASS] rangeQuery()\n";
}

void testUpdateMBRAndMarkDirty() {
    auto root = std::make_shared<RTreeNode>(true, 3);

    Trajectory t = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t");
    root->insertRecursive(t);

    BoundingBox3D mbrBefore = root->getMBR();
    root->markDirty(); // mark dirty recursively

    // Force update MBR
    BoundingBox3D mbrAfter = root->getMBR();

    assert(mbrAfter.volume() >= mbrBefore.volume());

    cout << "[PASS] updateMBR() and markDirty()\n";
}

void testSplitLeaf() {
    auto leaf = std::make_shared<RTreeNode>(true, 3);

    // Add 5 entries to force split (maxEntries=3)
    for (int i = 0; i < 5; ++i) {
        Trajectory t = createTrajectory({Point3D(i * 1.0f, i * 1.0f, "2023-01-01T00:0" + to_string(i) + ":00Z")}, "t" + to_string(i));
        leaf->insertLeaf(t.computeBoundingBox(), std::make_shared<Trajectory>(t));
    }

    auto [left, right] = leaf->splitLeaf();
    assert(left->getLeafEntries().size() > 0);
    assert(right->getLeafEntries().size() > 0);
    assert(left->getLeafEntries().size() + right->getLeafEntries().size() == 5);

    cout << "[PASS] splitLeaf()\n";
}

void testSplitInternal() {
    auto internal = std::make_shared<RTreeNode>(false, 3);

    for (int i = 0; i < 5; ++i) {
        auto child = std::make_shared<RTreeNode>(true, 3);
        BoundingBox3D box;
        box.expandToInclude(Point3D(i * 2.0f, i * 2.0f, "2023-01-01T00:0" + to_string(i) + ":00Z"));
        internal->insertChild(box, child);
    }

    auto [left, right] = internal->splitInternal();
    assert(left->getChildEntries().size() > 0);
    assert(right->getChildEntries().size() > 0);
    assert(left->getChildEntries().size() + right->getChildEntries().size() == 5);

    cout << "[PASS] splitInternal()\n";
}

void testDeleteTrajectoryAndCondenseTree() {
    auto root = std::make_shared<RTreeNode>(true, 3);

    Trajectory t1 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");
    Trajectory t2 = createTrajectory({Point3D(1, 1, "2023-01-01T00:01:00Z")}, "t2");
    root->insertRecursive(t1);
    root->insertRecursive(t2);

    bool deleted = root->deleteTrajectory("t1");
    assert(deleted);

    bool deletedAgain = root->deleteTrajectory("not_exist");
    assert(!deletedAgain);

    cout << "[PASS] deleteTrajectory() and condenseTree()\n";
}

void testUpdateTrajectory() {
    auto root = std::make_shared<RTreeNode>(true, 3);

    Trajectory t1 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");
    root->insertRecursive(t1);

    Trajectory t1_updated = createTrajectory({Point3D(5, 5, "2023-01-01T00:05:00Z")}, "t1");
    bool updated = root->updateTrajectory(t1_updated);
    assert(updated);

    Trajectory t_not_exist = createTrajectory({Point3D(1, 1, "2023-01-01T00:01:00Z")}, "tX");
    bool updatedFail = root->updateTrajectory(t_not_exist);
    assert(!updatedFail);

    cout << "[PASS] updateTrajectory()\n";
}

void testFindSimilar() {
    auto root = std::make_shared<RTreeNode>(true, 3);

    Trajectory t1 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");
    Trajectory t2 = createTrajectory({Point3D(5, 5, "2023-01-01T00:05:00Z")}, "t2");
    root->insertRecursive(t1);
    root->insertRecursive(t2);

    vector<Trajectory> results;
    Trajectory query = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "tQ");
    root->findSimilar(query, 1.0f, results);

    // t1 should be found (distance <= 1), t2 not
    assert(results.size() == 1);
    assert(results[0].getId() == "t1");

    cout << "[PASS] findSimilar()\n";
}

void testKNearestNeighbors() {
    // Start with an empty root (leaf, capacity 3)
    std::shared_ptr<RTreeNode> root = std::make_shared<RTreeNode>(true, 3);

    for (int i = 0; i < 5; ++i) {
        Trajectory t = createTrajectory(
            {Point3D(float(i), float(i), "2023-01-01T00:0" + std::to_string(i) + ":00Z")},
            "t" + std::to_string(i)
        );

        auto splitResult = root->insertRecursive(t);

        // If a split occurred, insertRecursive returns a pair of nodes
        if (splitResult.first && splitResult.second) {
            // Create a new root as an internal node
            auto newRoot = std::make_shared<RTreeNode>(false, 3);
            newRoot->insertChild(splitResult.first->getMBR(), splitResult.first);
            newRoot->insertChild(splitResult.second->getMBR(), splitResult.second);
            root = newRoot; // update root
        }
    }

    // Query trajectory
    Trajectory query = createTrajectory(
        {Point3D(1.1f, 1.1f, "2023-01-01T00:01:10Z")},
        "q"
    );

    // Ask for 3 nearest neighbors
    std::vector<Trajectory> knn = root->kNearestNeighbors(query, 3);

    assert(knn.size() == 3);
    std::cout << "[PASS] kNearestNeighbors() returned " << knn.size() << " results\n";
}

void testToJson() {
    auto root = std::make_shared<RTreeNode>(true, 3);

    Trajectory t1 = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t1");
    root->insertRecursive(t1);

    auto j = root->to_json();

    assert(!j.empty());
    assert(j.contains("isLeaf"));  
    if (root->isLeafNode()) {
        assert(j.contains("entries"));  
    } else {
        assert(j.contains("children"));  
    }

    cout << "[PASS] to_json()\n";
}


void testInsertLeafEntry() {
    auto leaf = std::make_shared<RTreeNode>(true, 3);

    Trajectory t = createTrajectory({Point3D(0, 0, "2023-01-01T00:00:00Z")}, "t");
    leaf->insertLeaf(t.computeBoundingBox(), std::make_shared<Trajectory>(t));
    
    auto entries = leaf->getLeafEntries();
    assert(entries.size() == 1);
    assert(entries[0].second->getId() == "t");

    cout << "[PASS] insertLeafEntry() and getLeafEntries()\n";
}

void testGetChildEntriesAndRecomputeMBRs() {
    auto internal = std::make_shared<RTreeNode>(false, 3);

    for (int i = 0; i < 2; ++i) {
        auto child = std::make_shared<RTreeNode>(true, 3);
        BoundingBox3D box;
        box.expandToInclude(Point3D(i, i, "2023-01-01T00:0" + to_string(i) + ":00Z"));
        internal->insertChild(box, child);
    }

    auto children = internal->getChildEntries();
    assert(children.size() == 2);

    internal->updateMBR();
    BoundingBox3D mbr = internal->getMBR();
    assert(mbr.getMinX() <= 0 && mbr.getMaxX() >= 1);

    cout << "[PASS] getChildEntries() and updateMBR()\n";
}

int main() {
    testPoint3DCreationAndEquality();
    testPoint3DDistance();
    testBoundingBoxCreationAndExpansion();
    testBoundingBoxVolumeAndIntersection();
    testTrajectoryCreationAndMBR();
    testTrajectoryDistanceAndEquality();
    testConstructorAndIsLeaf();
    testInsertAndNeedsSplit();
    testChooseSubtree();
    testRangeQuery();
    testUpdateMBRAndMarkDirty();
    testSplitLeaf();
    testSplitInternal();
    testDeleteTrajectoryAndCondenseTree();
    testUpdateTrajectory();
    testFindSimilar();
    testKNearestNeighbors();
    testToJson();
    testInsertLeafEntry();
    testGetChildEntriesAndRecomputeMBRs();

    cout << "\nAll tests passed successfully.\n";
    return 0;
}
