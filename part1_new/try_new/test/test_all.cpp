#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include "../include/point3D.h"
#include "../include/bbox3D.h"
#include "../include/trajectory.h"
#include "../include/RTree.h"
#include "../include/RTreeNode.h"

using namespace std;

void testPoint3D() {
    Point3D p(0.3f, 0.7f, 1000);
    assert(p.getX() == 0.3f);
    assert(p.getY() == 0.7f);
    assert(p.getT() == 1000);
    p.print();
}

void testBoundingBox3DBasic() {
    Point3D p1(0.1f, 0.2f, 100);
    Point3D p2(0.4f, 0.6f, 300);
    BoundingBox3D box;
    box.expandToInclude(p1);
    box.expandToInclude(p2);

    assert(box.getMinX() == 0.1f);
    assert(box.getMaxY() == 0.6f);
    assert(box.contains(p1));
    assert(box.contains(p2));
    assert(box.volume() > 0);
    std::cout << "Bounding Box volume: " << box.volume() << "\n";
}

void testBoundingBox3DIntersects() {
    BoundingBox3D b1(0.0f, 0.0f, 0, 1.0f, 1.0f, 1000);
    BoundingBox3D b2(0.5f, 0.5f, 500, 1.5f, 1.5f, 1500);
    BoundingBox3D b3(1.1f, 1.1f, 1100, 2.0f, 2.0f, 2000);

    assert(b1.intersects(b2));
    assert(!b1.intersects(b3));
    assert(b2.intersects(b3));
}

void testTrajectory() {
    std::vector<Point3D> points = {
        Point3D(0.1f, 0.2f, 100),
        Point3D(0.4f, 0.6f, 300),
        Point3D(0.3f, 0.5f, 200)
    };
    Trajectory traj(points, "t1");
    BoundingBox3D box = traj.computeBoundingBox();

    assert(box.contains(points[0]));
    assert(box.contains(points[2]));
    std::cout << "Trajectory bounding box volume: " << box.volume() << "\n";
}

void testRTreeInsertAndQuery() {
    RTree tree(4);

    std::vector<Point3D> traj1_points = {
        Point3D(0.1f, 0.1f, 100),
        Point3D(0.2f, 0.2f, 150)
    };
    std::vector<Point3D> traj2_points = {
        Point3D(0.6f, 0.7f, 400),
        Point3D(0.9f, 0.95f, 500)
    };

    Trajectory traj1(traj1_points, "veh1");
    Trajectory traj2(traj2_points, "veh2");

    tree.insert(traj1);
    tree.insert(traj2);

    BoundingBox3D queryBox(0.0f, 0.0f, 0, 0.3f, 0.3f, 200);
    auto results = tree.rangeQuery(queryBox);

    assert(results.size() == 1);
    assert(results[0].id == "veh1");
    std::cout << "Range query result size: " << results.size() << "\n";
}

void testSplitAndChooseSubtreeBehavior() {
    std::cout << "\n--- Testing split and chooseSubtree behavior ---\n";

    int maxEntries = 2;
    auto root = std::make_shared<RTreeNode>(false, maxEntries);

    for (int i = 0; i < 4; ++i) {
        auto child = std::make_shared<RTreeNode>(true, maxEntries);
        std::vector<Point3D> pts = {
            Point3D(0.1f * i, 0.1f * i, 100 * i),
            Point3D(0.1f * i + 0.02f, 0.1f * i + 0.02f, 100 * i + 50)
        };
        Trajectory t(pts, "veh" + std::to_string(i));
        child->insertRecursive(t);

        BoundingBox3D childMBR = t.computeBoundingBox();
        root->insert(childMBR, child);

        std::cout << "Inserted child " << i << " with MBR: " << childMBR.to_json().dump() << "\n";
    }

    // Now the root should need a split
    assert(root->needsSplit());
    auto [left, right] = root->splitInternal();
    std::cout << "Root split into left and right internal nodes.\n";
    std::cout << "Left MBR: " << left->getMBR().to_json().dump() << "\n";
    std::cout << "Right MBR: " << right->getMBR().to_json().dump() << "\n";

    // Testing chooseSubtree logic
    RTreeNode newRoot(false, maxEntries);
    newRoot.insert(left->getMBR(), left);
    newRoot.insert(right->getMBR(), right);

    BoundingBox3D testBox(0.05f, 0.05f, 200, 0.06f, 0.06f, 250);
    int chosenIndex = newRoot.chooseSubtree(testBox);
    std::cout << "ChooseSubtree picked child: " << chosenIndex << "\n";

    assert(chosenIndex == 0 || chosenIndex == 1);
    std::cout << "--- chooseSubtree test passed ---\n";
}

void testRTreeBulkInsertAndSplit() {
    RTree tree(2);  // low maxEntries to force splits

    for (int i = 0; i < 10; ++i) {
        std::vector<Point3D> pts = {
            Point3D(0.1f * i, 0.1f * i, 100 * i),
            Point3D(0.1f * i + 0.05f, 0.1f * i + 0.05f, 100 * i + 50)
        };
        tree.insert(Trajectory(pts, "veh" + std::to_string(i)));
    }

    BoundingBox3D queryAll(0.0f, 0.0f, 0, 1.0f, 1.0f, 1000);
    auto results = tree.rangeQuery(queryAll);

    assert(results.size() == 10);
    std::cout << "RTree handled 10 inserts and returned " << results.size() << " results.\n";
    tree.exportToJSON("../rtree_output_test.json");
}

int main() {
    cout << "\n=====================\n";
    cout << "Running R-tree Tests\n";
    cout << "=====================\n";

    testPoint3D();
    testBoundingBox3DBasic();
    testBoundingBox3DIntersects();
    testTrajectory();
    testRTreeInsertAndQuery();
    testSplitAndChooseSubtreeBehavior();
    testRTreeBulkInsertAndSplit();

    cout << "\n✅ All tests passed successfully!\n";
    return 0;
}
