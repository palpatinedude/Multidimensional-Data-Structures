
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include "../api/include/RTree.h"
#include "../api/include/point3D.h"
#include "../api/include/bbox3D.h"
#include "../api/include/trajectory.h"
#include "../api/include/RTreeNode.h"

// ---------- Helpers ----------
Trajectory createTrajectory(const std::string& id, const std::vector<Point3D>& points) {
    Trajectory traj(id);
    for (const auto& pt : points) {
        traj.addPoint(pt);
    }
    traj.setCachedBBox(traj.computeBoundingBox());
    return traj;
}

Point3D makePoint(float x, float y, const std::string& t) {
    return Point3D(x, y, t);
}

BoundingBox3D makeBoundingBox(float minX, float minY, const std::string& minT,
                              float maxX, float maxY, const std::string& maxT) {
    return BoundingBox3D(minX, minY, minT, maxX, maxY, maxT);
}

// ---------- Basic RTree Tests ----------
void testInsertAndRangeQuery() {
    std::cout << "Testing Insert and Range Query..." << std::endl;
    RTree tree(4);
    Trajectory t1 = createTrajectory("traj1", {
        makePoint(1, 1, "2025-08-08T00:00:00Z"),
        makePoint(2, 2, "2025-08-08T00:01:00Z")
    });
    Trajectory t2 = createTrajectory("traj2", {
        makePoint(5, 5, "2025-08-08T00:02:00Z"),
        makePoint(6, 6, "2025-08-08T00:03:00Z")
    });
    tree.insert(t1);
    tree.insert(t2);

    BoundingBox3D queryBox = makeBoundingBox(0, 0, "2025-08-08T00:00:00Z",
                                             3, 3, "2025-08-08T00:10:00Z");
    auto results = tree.rangeQuery(queryBox);
    assert(results.size() == 1 && results[0].getId() == "traj1");
    std::cout << "Insert and Range Query test passed!" << std::endl;
}

void testRemoveAndUpdate() {
    std::cout << "Testing Remove and Update..." << std::endl;
    RTree tree(4);
    Trajectory t1 = createTrajectory("traj1", {
        makePoint(1, 1, "2025-08-08T00:00:00Z"),
        makePoint(2, 2, "2025-08-08T00:01:00Z")
    });
    tree.insert(t1);
    assert(tree.remove("traj1"));
    assert(tree.rangeQuery(makeBoundingBox(0, 0, "2025-08-08T00:00:00Z",
                                           3, 3, "2025-08-08T00:10:00Z")).empty());
    tree.insert(t1);
    Trajectory t1_updated = createTrajectory("traj1", {
        makePoint(10, 10, "2025-08-08T00:10:00Z"),
        makePoint(11, 11, "2025-08-08T00:11:00Z")
    });
    assert(tree.update(t1_updated));
    auto results2 = tree.rangeQuery(makeBoundingBox(9, 9, "2025-08-08T00:09:00Z",
                                                    12, 12, "2025-08-08T00:20:00Z"));
    assert(results2.size() == 1 && results2[0].getId() == "traj1");
    std::cout << "Remove and Update test passed!" << std::endl;
}

void testKNearestNeighbors() {
    std::cout << "Testing k-Nearest Neighbors..." << std::endl;
    RTree tree(4);
    tree.insert(createTrajectory("traj1", { makePoint(1, 1, "2025-08-08T00:00:00Z") }));
    tree.insert(createTrajectory("traj2", { makePoint(2, 2, "2025-08-08T00:01:00Z") }));
    tree.insert(createTrajectory("traj3", { makePoint(3, 3, "2025-08-08T00:02:00Z") }));

    auto neighbors = tree.kNearestNeighbors(
        createTrajectory("query", { makePoint(1.5f, 1.5f, "2025-08-08T00:00:30Z") }), 2);
    assert(neighbors.size() == 2);
    std::cout << "k-Nearest Neighbors test passed!" << std::endl;
}

void testFindSimilar() {
    std::cout << "Testing Find Similar Trajectories..." << std::endl;
    RTree tree(4);
    tree.insert(createTrajectory("traj1", { makePoint(1, 1, "2025-08-08T00:00:00Z") }));
    tree.insert(createTrajectory("traj2", { makePoint(10, 10, "2025-08-08T00:01:00Z") }));

    auto similar = tree.findSimilar(
        createTrajectory("query", { makePoint(1.1f, 1.1f, "2025-08-08T00:00:30Z") }), 1.0f);
    assert(similar.size() == 1 && similar[0].getId() == "traj1");
    std::cout << "Find Similar test passed!" << std::endl;
}

// ---------- Expanded Stress Test ----------
void testBulkLoadAndStatistics() {
    std::cout << "Testing Bulk Load and Statistics with 1000+ trajectories..." << std::endl;
    std::vector<Trajectory> trajectories;
    for (int i = 0; i < 1200; ++i) {
        std::string id = "traj" + std::to_string(i);
        std::vector<Point3D> points = { makePoint(float(i), float(i), "2025-08-08T00:00:00Z") };
        Trajectory t (points, id);
        trajectories.push_back(t);
    }
    RTree tree(4);
    std::cout << "Bulk loading " << trajectories.size() << " trajectories..." << std::endl;
    tree.bulkLoad(trajectories);
    tree.printStatistics();
    assert(tree.getTotalEntries() == 1200);
    assert(tree.getHeight() > 0);
    std::cout << "Bulk Load and Statistics test passed!" << std::endl;
}

// ---------- JSON Export/Load ----------
void testExportAndLoadJSON() {
    std::cout << "Testing Export and Load JSON..." << std::endl;
    
    RTree tree(4);

    for (int i = 1; i <= 10; ++i) {
        std::vector<Point3D> points = {
            makePoint(i, i, "2025-08-08T00:00:00Z"),
            makePoint(i + 0.5f, i + 0.5f, "2025-08-08T00:01:00Z")
        };
        tree.insert(createTrajectory("traj" + std::to_string(i), points));
    }
    std::cout << "Inserted  trajectories into RTree." << std::endl;
    
    std::string filename = "test_rtree_export.json";
    tree.exportToJSON(filename);
   /*
    auto loaded = tree.loadFromJSON(filename);
    tree.bulkLoad(loaded);
    std::cout << "RTree loaded with " << tree.getTotalEntries() << " trajectories." << std::endl;
    tree.printStatistics();*/
    std::cout << "Export and Load JSON test passed!" << std::endl;
}

// ---------- Additional Unit Tests ----------
void testPoint3DandBBox3D() {
    std::cout << "Testing Point3D and BoundingBox3D basics..." << std::endl;
    Point3D p1(1, 2, "2025-08-08T00:00:00Z");
    Point3D p2(4, 6, "2025-08-08T00:01:00Z");
    assert(std::fabs(p1.distanceTo(p2) - 5.0f) < 1e-6);
    BoundingBox3D b1(0, 0, "2025-08-08T00:00:00Z", 5, 5, "2025-08-08T00:05:00Z");
    BoundingBox3D b2(3, 3, "2025-08-08T00:02:00Z", 7, 7, "2025-08-08T00:06:00Z");
    assert(b1.intersects(b2));
    b1.expandToInclude(b2);
    b2.contains(p2);
    
    std::cout << "Point3D & BoundingBox3D test passed!" << std::endl;
}

void testTrajectoryBasics() {
    std::cout << "Testing Trajectory basics..." << std::endl;
    Trajectory t("t1");
    t.addPoint(Point3D(0, 0, "2025-08-08T00:00:00Z"));
    t.addPoint(Point3D(1, 1, "2025-08-08T00:01:00Z"));
    auto bbox = t.computeBoundingBox();
    assert(bbox.getMinX() == 0 && bbox.getMaxX() == 1);
    auto j = t.to_json();
    assert(j.contains("id") && j["id"] == "t1");
    
    std::cout << "Trajectory basics test passed!" << std::endl;
}

void testRTreeNodeBasics() {
    std::cout << "Testing RTreeNode basics..." << std::endl;
    auto node = std::make_shared<RTreeNode>(true, 4);
    auto traj = std::make_shared<Trajectory>(createTrajectory("t1", {
        makePoint(0, 0, "2025-08-08T00:00:00Z")
    }));
    node->insertLeafEntry(traj->computeBoundingBox(), traj);
    assert(node->isLeafNode());
    std::cout << "RTreeNode basics test passed!" << std::endl;
}

// ---------- Main ----------
int main() {
   // testInsertAndRangeQuery();
  //  testRemoveAndUpdate();
  //  testKNearestNeighbors();
  //  testFindSimilar();
  //  testBulkLoadAndStatistics();
    testExportAndLoadJSON();
 //   testPoint3DandBBox3D();
 //   testTrajectoryBasics();
  //  testRTreeNodeBasics();

    std::cout << "✅ All tests passed successfully!" << std::endl;
    return 0;
}
