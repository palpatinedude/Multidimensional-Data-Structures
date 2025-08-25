#include <iostream>
#include <vector>
#include <string>
#include "../api/include/point3D.h"
#include "../api/include/bbox3D.h"
#include "../api/include/trajectory.h"
#include "../api/include/RTreeNode.h"
#include "../api/include/RTree.h"

// -------------------- Point3D Tests --------------------
void testPoint3D() {
    std::cout << "\n--- Testing Point3D ---\n";

    Point3D p1;  
    Point3D p2(10.0f, 20.0f, "2025-08-16T12:00:00");
    Point3D p3 = p2; 
    Point3D p4 = std::move(p2); 

    p1.print();
    p3.print();
    p4.print();

    std::cout << "Distance squared p3 -> p4: " << p3.distanceSquaredTo(p4) << "\n";
    std::cout << "Distance p3 -> p4: " << p3.distanceTo(p4) << "\n";
    std::cout << "JSON: " << p3.to_json().dump() << "\n";
    p3.validate();
}

// -------------------- BoundingBox3D Tests --------------------
void testBoundingBox3D() {
    std::cout << "\n--- Testing BoundingBox3D ---\n";

    BoundingBox3D box1;
    Point3D p1(1.0f, 2.0f, "2025-08-16T12:00:00");
    Point3D p2(5.0f, 6.0f, "2025-08-16T12:05:00");

    box1.expandToInclude(p1);
    box1.expandToInclude(p2);
    box1.print();

    BoundingBox3D box2(0.0f, 1.0f, "2025-08-16T11:50:00",
                       3.0f, 4.0f, "2025-08-16T12:10:00");
    box1.expandToInclude(box2);
    box1.print();

    std::cout << "Volume: " << box1.volume() << "\n";
    std::cout << "Intersects box2: " << box1.intersects(box2) << "\n";
    std::cout << "Contains p1: " << box1.contains(p1) << "\n";
    std::cout << "Distance squared to box2: " << box1.distanceSquaredTo(box2) << "\n";
    std::cout << "Euclidean distance to box2: " << box1.distanceTo(box2) << "\n";
    std::cout << "JSON: " << box1.to_json().dump() << "\n";
}

// -------------------- Trajectory Tests --------------------
void testTrajectory() {
    std::cout << "--- Testing Trajectory ---\n";

    // Create trajectories using string IDs
    Trajectory t1("1");
    Trajectory t2("2");
    Trajectory t3("3");

    // Print basic info
    std::vector<Trajectory> trajs = {t1, t2, t3};
    for (auto &t : trajs) {
        std::cout << "Trajectory ID: " << t.getId()
                  << ", #points: " << t.getPoints().size() << "\n";
    }

    std::cout << "Trajectory test completed.\n\n";
}


// -------------------- RTreeNode Tests --------------------

void testRTreeNode() {
    std::cout << "--- Testing RTreeNode ---\n";

    // Create a leaf node
    RTreeNode leaf(true,3);

    // Create trajectories
    Trajectory t1("1");
    Trajectory t2("2");
    Trajectory t3("3");

    // Insert trajectories into the leaf
    leaf.insertLeafEntry(t1.getBoundingBox(), std::make_shared<Trajectory>(t1));
    leaf.insertLeafEntry(t2.getBoundingBox(), std::make_shared<Trajectory>(t2));
    leaf.insertLeafEntry(t3.getBoundingBox(), std::make_shared<Trajectory>(t3));

    // Recompute the MBR of the leaf
    leaf.recomputeMBRs();

    // Print enlargement of adding a new trajectory
    Trajectory tNew("new");
    float enlarge = leaf.enlargement(leaf.getMBR(), tNew.getBoundingBox());
    std::cout << "Enlargement to add new trajectory: " << enlarge << "\n";

    // Print node info
    std::cout << "Leaf is leaf? " << (leaf.isLeafNode() ? "Yes" : "No") << "\n";
    std::cout << "Number of entries: " << leaf.getLeafEntries().size() << "\n";

    std::cout << "RTreeNode test completed.\n\n";
}


// -------------------- RTree Tests --------------------
void testRTree() {
    std::cout << "\n--- Testing RTree ---\n";

    RTree tree(2);

    Trajectory t1({Point3D(1,1,"2025-08-16T12:00:00")}, "1");
    Trajectory t2({Point3D(10,10,"2025-08-16T12:10:00")}, "2");
    Trajectory t3({Point3D(5,5,"2025-08-16T12:20:00")}, "3");
    Trajectory t4({Point3D(12,12,"2025-08-16T12:30:00")}, "4");
    Trajectory t5({Point3D(6,6,"2025-08-16T12:40:00")}, "5");

    // Insert trajectories (multiple splits)
    tree.insert(t1);
    tree.insert(t2);
    tree.insert(t3);
    tree.insert(t4);
    tree.insert(t5);


    // Bulk load with overlapping MBRs
    std::vector<Trajectory> trajs = {t1,t2,t3,t4,t5};
    RTree bulkTree(3);
    bulkTree.bulkLoad(trajs);


    // Search with overlapping MBR
    BoundingBox3D searchBox(0,0,"2025-08-16T11:50:00",6,6,"2025-08-16T12:25:00");
    std::vector<Trajectory> found = bulkTree.rangeQuery(searchBox);
    std::cout << "\nTrajectories intersecting search box:\n";
    for(auto &traj : found) std::cout << traj.to_json().dump(4) << "\n";


    // Delete a trajectory
    bulkTree.remove(t2.getId());
    std::cout << "\nAfter deletion of trajectory 2:\n";
    bulkTree.exportToJSON("after_deletion.json");

    // Update trajectory
    Trajectory tUpdate("3");
    tUpdate.addPoint(Point3D(6,6,"2025-08-16T12:50:00"));
    bulkTree.update(tUpdate);
    std::cout << "\nAfter updating trajectory 3:\n";
    bulkTree.exportToJSON("updated_tree.json");

    // Empty search
    BoundingBox3D emptySearch(100,100,"2025-08-16T15:00:00",200,200,"2025-08-16T16:00:00");
    std::vector<Trajectory> emptyFound = bulkTree.rangeQuery(emptySearch);
    std::cout << "Search found " << emptyFound.size() << " trajectories (expected 0).\n";

    // RTree JSON
     bulkTree.exportToJSON("final.json");
}

void testInsertAndPrint() {
    std::cout << "\n===== TEST: Insert & Print =====\n";
    RTree tree(2);

    Trajectory t1("traj1");
    t1.addPoint(Point3D(0,0,"2024-08-16T12:50:00"));
    t1.addPoint(Point3D(1,1,"2024-08-16T12:51:00"));

    Trajectory t2("traj2");
    t2.addPoint(Point3D(2,2,"2023-08-16T12:52:00"));
    t2.addPoint(Point3D(3,3,"2023-08-16T12:53:00"));

    Trajectory t3("traj3");
    t3.addPoint(Point3D(4,4,"2022-08-16T12:54:00"));
    t3.addPoint(Point3D(5,5,"2022-08-16T12:55:00"));

    tree.insert(t1);
    tree.printStatistics();
    tree.insert(t2);
    tree.printStatistics();
    tree.insert(t3); // should cause a split
    tree.printStatistics();

    std::cout << "\nTree structure after inserts:\n";
    tree.exportToJSON("test_rtree_structure.json");
}

// -------------------- Bulk Load from JSON Test --------------------
void testBulkLoadFromJSON() {
    std::cout << "\n===== TEST: Bulk Load From JSON =====\n";

    try {
        // Step 1: Load trajectories from JSON file
        std::string inputFile = "trajectories_input.json";  // replace with your file
        std::vector<Trajectory> trajectories = RTree::loadFromJSON(inputFile);

        std::cout << "Loaded " << trajectories.size() << " trajectories from file.\n";

        // Step 2: Bulk load into RTree
        RTree tree(3);  // choose fanout (maxEntries per node)
        tree.bulkLoad(trajectories);
        
        // Step 3: Export RTree to JSON for verification
        std::string outputFile = "bulkloaded_tree.json";
        tree.exportToJSON(outputFile);
        std::cout << "Exported RTree structure to: " << outputFile << "\n";

        // Step 4: Print statistics
        tree.printStatistics();
    } 
    catch (const std::exception& ex) {
        std::cerr << "Test failed: " << ex.what() << "\n";
    }
}


// -------------------- Main --------------------
int main() {
  //  testPoint3D();
  //  testBoundingBox3D();
   // testTrajectory();
   // testRTreeNode();
   // testRTree();
    //testInsertAndPrint();
    testBulkLoadFromJSON();
    return 0;
}
