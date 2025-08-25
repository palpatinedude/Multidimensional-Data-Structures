// test_updateTrajectory.cpp
#include "../api/include/RTree.h"
#include "../api/include/trajectory.h"
#include "../api/include/bbox3D.h"
#include "../api/include/point3D.h"
#include "../api/include/RTreeNode.h"
#include <cassert>
#include <iostream>

int main() {
    std::cout << "=== RTree Update Trajectory Test ===\n";

    // 1. Create RTree
    RTree tree(4); // max 4 entries per node

    // 2. Create and insert trajectories
    Trajectory t1("traj1");
    t1.addPoint(Point3D(0.0f, 0.0f, "2025-08-20T08:00:00"));
    t1.addPoint(Point3D(1.0f, 1.0f, "2025-08-20T08:05:00"));

    Trajectory t2("traj2");
    t2.addPoint(Point3D(5.0f, 5.0f, "2025-08-20T09:00:00"));
    t2.addPoint(Point3D(6.0f, 6.0f, "2025-08-20T09:05:00"));

    tree.insert(t1);
    tree.insert(t2);

    std::cout << "Initial trajectories inserted.\n";

    // 3. Verify tree contains trajectories
    auto allTraj = tree.getAllLeafTrajectories();
    assert(allTraj.size() == 2);

    // 4. Update trajectory within same bounding box
    Trajectory t1_update("traj1");
    t1_update.addPoint(Point3D(0.1f, 0.1f, "2025-08-20T08:01:00"));
    t1_update.addPoint(Point3D(1.1f, 1.1f, "2025-08-20T08:06:00"));

    bool updated = tree.update(t1_update);
    assert(updated);

    std::cout << "Trajectory 'traj1' updated within same leaf.\n";

    // Verify update
    allTraj = tree.getAllLeafTrajectories();
    bool foundUpdate = false;
    for (const auto& traj : allTraj) {
        if (traj.getId() == "traj1") {
            const auto& points = traj.getPoints();
            if (points[0].getX() == 0.1f && points[1].getX() == 1.1f) {
                foundUpdate = true;
                break;
            }
        }
    }
    assert(foundUpdate);

    // 5. Update trajectory that requires moving leaf
    Trajectory t2_update("traj2");
    t2_update.addPoint(Point3D(100.0f, 100.0f, "2025-08-20T10:00:00")); // Move far away
    t2_update.addPoint(Point3D(101.0f, 101.0f, "2025-08-20T10:05:00"));

    updated = tree.update(t2_update);
    assert(updated);

    std::cout << "Trajectory 'traj2' updated and repositioned in tree.\n";

    // 6. Verify update using intersects instead of contains
    allTraj = tree.getAllLeafTrajectories();
    foundUpdate = false;
    BoundingBox3D t2_box = t2_update.getBoundingBox(); 
    for (const auto& traj : allTraj) {
        BoundingBox3D leaf_box = traj.getBoundingBox(); 
        if (traj.getId() == "traj2" && leaf_box.intersects(t2_box)) {
            const auto& points = traj.getPoints();
            if (points[0].getX() == 100.0f && points[1].getX() == 101.0f) {
                foundUpdate = true;
                break;
            }
        }
    }
    assert(foundUpdate);

    std::cout << "All updates verified successfully.\n";

    // 7. Print statistics
    tree.printStatistics();

    std::cout << "=== Test Completed Successfully ===\n";
    return 0;
}
