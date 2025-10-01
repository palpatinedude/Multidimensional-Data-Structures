#include "../api/include/RTreeNode.h"
#include "../api/include/trajectory.h"
#include "../api/include/point3D.h"
#include "../api/include/bbox3D.h"
#include "../api/include/splitHelpers.inl" 
#include <memory>
#include <vector>
#include <algorithm>

using namespace splitHelpers;

int main() {
    std::cout << "=== Starting Enhanced RTreeNode + splitHelpers Tests ===\n";

    // ---------------- Create sample trajectories ----------------
    auto traj1 = std::make_shared<Trajectory>("traj1");
    traj1->addPoint(Point3D(0,0,1000));
    traj1->addPoint(Point3D(3,4,1100));

    auto traj2 = std::make_shared<Trajectory>("traj2");
    traj2->addPoint(Point3D(1,1,1020));
    traj2->addPoint(Point3D(4,5,1120));

    auto traj3 = std::make_shared<Trajectory>("traj3");
    traj3->addPoint(Point3D(-2,2,950));
    traj3->addPoint(Point3D(8,10,1300));

    auto traj4 = std::make_shared<Trajectory>("traj4");
    traj4->addPoint(Point3D(5,5,1050));
    traj4->addPoint(Point3D(6,6,1150));

    auto traj5 = std::make_shared<Trajectory>("traj5");
    traj5->addPoint(Point3D(-5,-5,900));
    traj5->addPoint(Point3D(0,0,950));

    auto traj6 = std::make_shared<Trajectory>("traj6");
    traj6->addPoint(Point3D(100,100,2000)); // Ensures non-zero volume

    // ----------------- Create leaf node with maxEntries=2 ----------------
    auto leafNode = std::make_shared<RTreeNode>(true, 2);
    leafNode->insertLeaf(traj1->getBoundingBox(), traj1);
    leafNode->insertLeaf(traj2->getBoundingBox(), traj2);

    std::cout << "Initial LeafNode MBR: " << leafNode->getMBR() << "\n";
    std::cout << "LeafNode needs split? " << (leafNode->needsSplit() ? "Yes" : "No") << "\n";

    // ----------------- Insert another trajectory to trigger split -----------------
    auto [leftSplit, rightSplit] = leafNode->insertRecursive(*traj3);
    std::cout << "Insert traj3 triggers split? " 
              << ((leftSplit && rightSplit) ? "Yes" : "No") << "\n";

    if (leftSplit && rightSplit) {
        std::cout << "Left Split MBR: " << leftSplit->getMBR() << "\n";
        std::cout << "Right Split MBR: " << rightSplit->getMBR() << "\n";
    }

    // ----------------- Direct splitLeaf test -----------------
    auto testLeaf = std::make_shared<RTreeNode>(true, 2);
    testLeaf->insertLeaf(traj1->getBoundingBox(), traj1);
    testLeaf->insertLeaf(traj2->getBoundingBox(), traj2);
    testLeaf->insertLeaf(traj3->getBoundingBox(), traj3);
    auto [splitA, splitB] = testLeaf->splitLeaf();
    std::cout << "Direct splitLeaf test MBRs:\n";
    std::cout << "SplitA MBR: " << splitA->getMBR() << "\n";
    std::cout << "SplitB MBR: " << splitB->getMBR() << "\n";

    // ----------------- Internal node test -----------------
    auto root = std::make_shared<RTreeNode>(false, 2);
    root->insertChild(splitA->getMBR(), splitA);
    root->insertChild(splitB->getMBR(), splitB);
    std::cout << "Root MBR after adding split leaves: " << root->getMBR() << "\n";

    root->insertRecursive(*traj4);
    root->insertRecursive(*traj5);
    root->insertRecursive(*traj6);
    std::cout << "Root MBR after adding traj4, traj5, traj6: " << root->getMBR() << "\n";
    std::cout << "Root needs split? " << (root->needsSplit() ? "Yes" : "No") << "\n";

    // ----------------- Range query test -----------------
    BoundingBox3D queryBox(-1, -1, 900, 5, 5, 1150);
    std::vector<Trajectory> rangeResults;
    root->rangeQuery(queryBox, rangeResults);
    std::cout << "Range query results (expected traj1, traj2, traj5): ";
    for (auto& t : rangeResults) std::cout << t.getId() << " ";
    std::cout << "\n";

    // ----------------- Similarity search -----------------
    std::vector<Trajectory> similarResults;
    root->findSimilar(*traj1, 5.0f, similarResults);
    std::cout << "Similarity search results to traj1 (threshold 5.0): ";
    for (auto& t : similarResults) std::cout << t.getId() << " ";
    std::cout << "\n";

    // ----------------- kNN search -----------------
    auto knnResults = root->kNearestNeighbors(*traj1, 3, 1.0f);
    std::cout << "kNN results for traj1 (k=3): ";
    for (auto& t : knnResults) std::cout << t.getId() << " ";
    std::cout << "\n";

    // ----------------- Update trajectory -----------------
    traj2->addPoint(Point3D(10,10,1400));
    bool updated = root->updateTrajectory(*traj2);
    std::cout << "Update traj2 success? " << (updated ? "Yes" : "No") << "\n";
    std::cout << "Root MBR after update: " << root->getMBR() << "\n";

    // ----------------- Delete trajectory -----------------
    bool deleted = root->deleteTrajectory("traj3");
    std::cout << "Delete traj3 success? " << (deleted ? "Yes" : "No") << "\n";
    std::cout << "Root MBR after deletion: " << root->getMBR() << "\n";

    // ----------------- Accessors tests -----------------
    std::cout << "Root is leaf? " << (root->isLeafNode() ? "Yes" : "No") << "\n";
    std::cout << "Root child entries count: " << root->getChildEntries().size() << "\n";

    // ----------------- Direct splitHelpers tests -----------------
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> testEntries = {
        {traj1->getBoundingBox(), traj1},
        {traj2->getBoundingBox(), traj2},
        {traj3->getBoundingBox(), traj3},
        {traj4->getBoundingBox(), traj4}
    };

    auto leftNode = std::make_shared<RTreeNode>(true, 2);
    auto rightNode = std::make_shared<RTreeNode>(true, 2);

    std::cout << "\n--- Testing splitHelpers.pickSeeds ---\n";
    auto [seed1, seed2] = pickSeeds(testEntries);
    std::cout << "Picked seeds: " << seed1 << " and " << seed2 << "\n";

    std::cout << "\n--- Testing splitHelpers.assignEntryToNode ---\n";
    assignEntryToNode(leftNode, testEntries[0]);
    assignEntryToNode(rightNode, testEntries[1]);
    std::cout << "Left node entries: " << leftNode->getLeafEntries().size()
              << ", Right node entries: " << rightNode->getLeafEntries().size() << "\n";

    std::cout << "\n--- Testing splitHelpers.assignForcedEntries ---\n";
    std::vector<bool> assigned(testEntries.size(), false);
    bool forced = assignForcedEntries(leftNode, rightNode, testEntries, assigned, 2);
    std::cout << "Forced assignment performed? " << (forced ? "Yes" : "No") << "\n";

    std::cout << "\n--- Testing splitHelpers.pickNextEntry ---\n";
    bool assignToLeft = false;
    int nextEntry = pickNextEntry(leftNode, rightNode, testEntries, assigned, assignToLeft);
    std::cout << "Next entry: " << nextEntry << " assigned to " << (assignToLeft ? "Left" : "Right") << "\n";

    std::cout << "\n--- Testing splitHelpers.quadraticSplitEntries ---\n";
    auto qLeft = std::make_shared<RTreeNode>(true, 2);
    auto qRight = std::make_shared<RTreeNode>(true, 2);
    quadraticSplitEntries(testEntries, qLeft, qRight, 2);
    std::cout << "Quadratic split result - Left MBR: " << qLeft->getMBR()
              << ", Right MBR: " << qRight->getMBR() << "\n";

    std::cout << "\n=== All RTreeNode + splitHelpers tests completed successfully ===\n";

    return 0;
}
