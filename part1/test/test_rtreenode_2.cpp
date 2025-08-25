#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "../api/include/RTreeNode.h"
#include "../api/include/bbox3D.h"
#include "../api/include/trajectory.h"


/*
// Helper to create a Trajectory from points
Trajectory makeTrajectory(const std::string& id,
                          const std::vector<std::tuple<double, double, std::string>>& points) {
    std::vector<Point3D> pts;
    for (auto& [x, y, t] : points) {
        pts.emplace_back(x, y, t);
    }
    return Trajectory(pts, id);
}

// ------------------- TEST FUNCTIONS -------------------

void testInsert(std::shared_ptr<RTreeNode>& root) {
    Trajectory t1 = makeTrajectory("1_1", {
        {12.34, 45.67, "2025-08-20T08:00:00"},
        {12.56, 45.89, "2025-08-20T08:10:00"}
    });
    Trajectory t2 = makeTrajectory("2_3", {
        {13.12, 46.01, "2025-08-20T09:00:00"},
        {13.50, 46.45, "2025-08-20T09:15:00"}
    });
    root->insertRecursive(t1);
    root->insertRecursive(t2);
    std::cout << "Insert test: leaf entries = " << root->getLeafEntries().size() << "\n";
}

void testSplitLeaf() {
    auto leaf = std::make_shared<RTreeNode>(true, 2);
    Trajectory t1 = makeTrajectory("1", {{0,0,"2025-08-20T08:00:00"}});
    Trajectory t2 = makeTrajectory("2", {{1,1,"2025-08-20T09:00:00"}});
    Trajectory t3 = makeTrajectory("3", {{2,2,"2025-08-20T10:00:00"}});
    leaf->insertLeaf(t1.getBoundingBox(), std::make_shared<Trajectory>(t1));
    leaf->insertLeaf(t2.getBoundingBox(), std::make_shared<Trajectory>(t2));
    leaf->insertLeaf(t3.getBoundingBox(), std::make_shared<Trajectory>(t3));
    auto [left, right] = leaf->splitLeaf();
    std::cout << "splitLeaf test: left=" << left->getLeafEntries().size()
              << ", right=" << right->getLeafEntries().size() << "\n";
}

void testSplitInternal() {
    auto internal = std::make_shared<RTreeNode>(false, 2);
    auto child1 = std::make_shared<RTreeNode>(true,2);
    auto child2 = std::make_shared<RTreeNode>(true,2);
    auto child3 = std::make_shared<RTreeNode>(true,2);

    BoundingBox3D b1(0,0,"2025-08-20T08:00:00",1,1,"2025-08-20T09:00:00");
    BoundingBox3D b2(1,1,"2025-08-20T09:00:00",2,2,"2025-08-20T10:00:00");
    BoundingBox3D b3(2,2,"2025-08-20T10:00:00",3,3,"2025-08-20T11:00:00");

    internal->insertChild(b1, child1);
    internal->insertChild(b2, child2);
    internal->insertChild(b3, child3);
    auto [left, right] = internal->splitInternal();
    std::cout << "splitInternal test: left=" << left->getChildEntries().size()
              << ", right=" << right->getChildEntries().size() << "\n";
}

void testRangeQuery(std::shared_ptr<RTreeNode>& root) {
    BoundingBox3D query(12,45,"2025-08-20T07:50:00",14,47,"2025-08-20T09:20:00");
    std::vector<Trajectory> results;
    root->rangeQuery(query, results);
    std::cout << "rangeQuery test: found " << results.size() << " trajectories\n";
}

void testDeleteUpdate(std::shared_ptr<RTreeNode>& root) {
    bool deleted = root->deleteTrajectory("2_3");
    std::cout << "deleteTrajectory test: deleted? " << (deleted?"yes":"no") << "\n";
    Trajectory t1_updated = makeTrajectory("1_1", {{12.6,45.9,"2025-08-20T08:20:00"}});
    bool updated = root->updateTrajectory(t1_updated);
    std::cout << "updateTrajectory test: updated? " << (updated?"yes":"no") << "\n";
}

void testFindSimilar(std::shared_ptr<RTreeNode>& root) {
    Trajectory query = makeTrajectory("q", {{12.5,45.8,"2025-08-20T08:05:00"}});
    std::vector<Trajectory> results;
    root->findSimilar(query,5.0f,results);
    std::cout << "findSimilar test: found " << results.size() << " trajectories\n";
}

void testKNN(std::shared_ptr<RTreeNode>& root) {
    Trajectory query = makeTrajectory("q", {{12.5,45.8,"2025-08-20T08:05:00"}});
    auto knn = root->kNearestNeighbors(query, 2);
    std::cout << "kNearestNeighbors test: found " << knn.size() << " trajectories\n";
}

void testDirtyAndMBR(std::shared_ptr<RTreeNode>& root) {
    root->markDirty();
    std::cout << "markDirty test executed\n";
    BoundingBox3D mbr = root->getMBR();
    std::cout << "getMBR test: min=(" << mbr.getMinX() << "," << mbr.getMinY() 
              << "), max=(" << mbr.getMaxX() << "," << mbr.getMaxY() << ")\n";
    root->recomputeMBRs();
    std::cout << "recomputeMBRs test executed\n";
}

void testOperatorsAndEmpty(std::shared_ptr<RTreeNode>& root) {
    auto clone = std::make_shared<RTreeNode>(*root);
    std::cout << "operator== test: " << ((*root==*clone)?"yes":"no") << "\n";
    std::cout << "operator!= test: " << ((*root!=*clone)?"yes":"no") << "\n";
    std::cout << "isEmpty test: " << (root->isEmpty()?"yes":"no") << "\n";
}

void testEnlargementAndChooseSubtree(std::shared_ptr<RTreeNode>& root) {
    BoundingBox3D box1(0,0,"t0",1,1,"t1");
    BoundingBox3D box2(1,1,"t1",2,2,"t2");
    float enlarge = root->enlargement(box1,box2);
    int idx = root->chooseSubtree(box2);
    std::cout << "enlargement test: " << enlarge << "\n";
    std::cout << "chooseSubtree test: " << idx << "\n";
}

void testJSON(std::shared_ptr<RTreeNode>& root) {
    json j = root->to_json();
    std::cout << "to_json test:\n" << j.dump(2) << "\n";
}

// ------------------- MAIN -------------------

int main() {
    try {
        auto root = std::make_shared<RTreeNode>(true, 2);

        testInsert(root);
        testSplitLeaf();
        testSplitInternal();
        testRangeQuery(root);
        testDeleteUpdate(root);
        testFindSimilar(root);
        testKNN(root);
        testDirtyAndMBR(root);
        testOperatorsAndEmpty(root);
        testEnlargementAndChooseSubtree(root);
        testJSON(root);

    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    return 0;
}
*/



// Helper to create a Trajectory from points
Trajectory makeTrajectory(const std::string& id,
                          const std::vector<std::tuple<double, double, std::string>>& points) {
    std::vector<Point3D> pts;
    for (auto& [x, y, t] : points) {
        pts.emplace_back(x, y, t);
    }
    return Trajectory(pts, id);
}

// ------------------- TEST FUNCTIONS -------------------

void testInsert(std::shared_ptr<RTreeNode>& root) {
    Trajectory t1 = makeTrajectory("1_1", {
        {12.34, 45.67, "2025-08-20T08:00:00"},
        {12.56, 45.89, "2025-08-20T08:10:00"}
    });
    Trajectory t2 = makeTrajectory("2_3", {
        {13.12, 46.01, "2025-08-20T09:00:00"},
        {13.50, 46.45, "2025-08-20T09:15:00"}
    });
    root->insertRecursive(t1);
    root->insertRecursive(t2);
    std::cout << "Insert test: leaf entries = " << root->getLeafEntries().size() << "\n";
}

void testSplitLeaf() {
    auto leaf = std::make_shared<RTreeNode>(true, 2);
    Trajectory t1 = makeTrajectory("1", {{0,0,"2025-08-20T08:00:00"}});
    Trajectory t2 = makeTrajectory("2", {{1,1,"2025-08-20T09:00:00"}});
    Trajectory t3 = makeTrajectory("3", {{2,2,"2025-08-20T10:00:00"}});
    leaf->insertLeaf(t1.getBoundingBox(), std::make_shared<Trajectory>(t1));
    leaf->insertLeaf(t2.getBoundingBox(), std::make_shared<Trajectory>(t2));
    leaf->insertLeaf(t3.getBoundingBox(), std::make_shared<Trajectory>(t3));
    auto [left, right] = leaf->splitLeaf();
    std::cout << "splitLeaf test: left=" << left->getLeafEntries().size()
              << ", right=" << right->getLeafEntries().size() << "\n";
}
/*
void testSplitInternal() {
    auto internal = std::make_shared<RTreeNode>(false, 2);
    auto child1 = std::make_shared<RTreeNode>(true,2);
    auto child2 = std::make_shared<RTreeNode>(true,2);
    auto child3 = std::make_shared<RTreeNode>(true,2);

    BoundingBox3D b1(0,0,"2025-08-20T08:00:00",1,1,"2025-08-20T09:00:00");
    BoundingBox3D b2(1,1,"2025-08-20T09:00:00",2,2,"2025-08-20T10:00:00");
    BoundingBox3D b3(2,2,"2025-08-20T10:00:00",3,3,"2025-08-20T11:00:00");

    internal->insertChild(b1, child1);
    internal->insertChild(b2, child2);
    internal->insertChild(b3, child3);
    auto [left, right] = internal->splitInternal();
    std::cout << "splitInternal test: left=" << left->getChildEntries().size()
              << ", right=" << right->getChildEntries().size() << "\n";
}*/

void testSplitInternal() {
    auto internal = std::make_shared<RTreeNode>(false, 2);

    // Create leaf children
    std::vector<std::shared_ptr<RTreeNode>> children;
    for (int i = 0; i < 12; i++) {
        children.push_back(std::make_shared<RTreeNode>(true, 2));
    }

    // Create bounding boxes (each one shifted by +1 in space & +1h in time)
    std::vector<BoundingBox3D> boxes;
    for (int i = 0; i < 12; i++) {
        int x1 = i, y1 = i;
        int x2 = i + 1, y2 = i + 1;

        std::string t1 = std::string("2025-08-20T") + 
                     (i < 10 ? "0" : "") + std::to_string(i) + ":00:00";
        std::string t2 = std::string("2025-08-20T") + 
                     (i+1 < 10 ? "0" : "") + std::to_string(i + 1) + ":00:00";

        boxes.emplace_back(x1, y1, t1, x2, y2, t2);
    }

    // Insert into internal node
    for (int i = 0; i < 12; i++) {
        internal->insertChild(boxes[i], children[i]);
    }

    // Perform split
    auto [left, right] = internal->splitInternal();

    std::cout << "splitInternal test: left="
              << left->getChildEntries().size()
              << ", right=" << right->getChildEntries().size()
              << "\n";
}


void testRangeQuery(std::shared_ptr<RTreeNode>& root) {
    BoundingBox3D query(12,45,"2025-08-20T07:50:00",14,47,"2025-08-20T09:20:00");
    std::vector<Trajectory> results;
    root->rangeQuery(query, results);
    std::cout << "rangeQuery test: found " << results.size() << " trajectories\n";
}

void testDeleteUpdate(std::shared_ptr<RTreeNode>& root) {
    bool deleted = root->deleteTrajectory("2_3");
    std::cout << "deleteTrajectory test: deleted? " << (deleted?"yes":"no") << "\n";
    Trajectory t1_updated = makeTrajectory("1_1", {{12.6,45.9,"2025-08-20T08:20:00"}});
    bool updated = root->updateTrajectory(t1_updated);
    std::cout << "updateTrajectory test: updated? " << (updated?"yes":"no") << "\n";
}

void testFindSimilar(std::shared_ptr<RTreeNode>& root) {
    // Meaningful test: query near updated trajectory
    Trajectory query = makeTrajectory("q", {{12.6,45.9,"2025-08-20T08:21:00"}});
    std::vector<Trajectory> results;
    root->findSimilar(query,0.5f,results); // small maxDistance ensures match
    std::cout << "findSimilar test: found " << results.size() << " trajectories\n";
}

void testKNN(std::shared_ptr<RTreeNode>& root) {
    Trajectory query = makeTrajectory("q", {{12.5,45.8,"2025-08-20T08:05:00"}});
    auto knn = root->kNearestNeighbors(query, 2);
    std::cout << "kNearestNeighbors test: found " << knn.size() << " trajectories\n";
}

void testDirtyAndMBR(std::shared_ptr<RTreeNode>& root) {
    root->markDirty();
    std::cout << "markDirty test executed\n";
    BoundingBox3D mbr = root->getMBR();
    std::cout << "getMBR test: min=(" << mbr.getMinX() << "," << mbr.getMinY() 
              << "), max=(" << mbr.getMaxX() << "," << mbr.getMaxY() << ")\n";
    root->recomputeMBRs();
    std::cout << "recomputeMBRs test executed\n";
}

void testOperatorsAndEmpty(std::shared_ptr<RTreeNode>& root) {
    auto clone = std::make_shared<RTreeNode>(*root);
    std::cout << "operator== test: " << ((*root==*clone)?"yes":"no") << "\n";
    std::cout << "operator!= test: " << ((*root!=*clone)?"yes":"no") << "\n";
    std::cout << "isEmpty test: " << (root->isEmpty()?"yes":"no") << "\n";
}

void testEnlargementAndChooseSubtree() {
    // Meaningful chooseSubtree test on internal node with children
    auto internal = std::make_shared<RTreeNode>(false, 3);
    auto child1 = std::make_shared<RTreeNode>(true,2);
    auto child2 = std::make_shared<RTreeNode>(true,2);
    BoundingBox3D b1(0,0,"2025-08-20T08:00:00",1,1,"2025-08-20T09:00:00");
    BoundingBox3D b2(1,1,"2025-08-20T09:00:00",2,2,"2025-08-20T10:00:00");
    internal->insertChild(b1, child1);
    internal->insertChild(b2, child2);

    BoundingBox3D newBox(0.5,0.5,"2025-08-20T08:30:00",1.5,1.5,"2025-08-20T09:30:00");
    float enlarge = internal->enlargement(b1,newBox);
    int idx = internal->chooseSubtree(newBox);
    std::cout << "enlargement test: " << enlarge << "\n";
    std::cout << "chooseSubtree test: " << idx << "\n";
}

void testJSON(std::shared_ptr<RTreeNode>& root) {
    json j = root->to_json();
    std::cout << "to_json test:\n" << j.dump(2) << "\n";
}

// ------------------- MAIN -------------------

int main() {
    try {
        auto root = std::make_shared<RTreeNode>(true, 2);

        testInsert(root);
        testSplitLeaf();
        testSplitInternal();
        testRangeQuery(root);
        testDeleteUpdate(root);
        testFindSimilar(root);
        testKNN(root);
        testDirtyAndMBR(root);
        testOperatorsAndEmpty(root);
        testEnlargementAndChooseSubtree();
        testJSON(root);

    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    return 0;
}
