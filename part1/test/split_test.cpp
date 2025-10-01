#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "../api/include/RTreeNode.h"
#include "../api/include/bbox3D.h"
#include "../api/include/trajectory.h"
#include "../api/include/splitHelpers.inl"

using namespace splitHelpers;

int main() {
    std::cout << "=== RTreeNode Split Function Tests ===\n";

    // Prepare sample leaf entries (BoundingBox3D + Trajectory)
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> leafEntries;

    for (int i = 0; i < 5; ++i) {
        BoundingBox3D box(0.0f + i, 0.0f + i, "2025-08-20T08:00:00",
                          1.0f + i, 1.0f + i, "2025-08-20T08:10:00");
        auto traj = std::make_shared<Trajectory>("traj_" + std::to_string(i));
        leafEntries.emplace_back(box, traj);
    }

    // ------------------ pickSeeds ------------------
    auto [s1, s2] = pickSeeds(leafEntries);
    std::cout << "pickSeeds indices: " << s1 << ", " << s2 << "\n";

    // ------------------ assignEntryToNode ------------------
    auto leftNode = std::make_shared<RTreeNode>(true, 4);
    auto rightNode = std::make_shared<RTreeNode>(true, 4);
    assignEntryToNode(leftNode, leafEntries[0]);
    assignEntryToNode(rightNode, leafEntries[1]);
    std::cout << "assignEntryToNode done. Left size: " << leftNode->getLeafEntries().size()
              << ", Right size: " << rightNode->getLeafEntries().size() << "\n";

    // ------------------ assignForcedEntries ------------------
    std::vector<bool> assigned(leafEntries.size(), false);
    assigned[0] = assigned[1] = true;
    bool forcedAssigned = assignForcedEntries(leftNode, rightNode, leafEntries, assigned, 3);
    std::cout << "assignForcedEntries returned: " << (forcedAssigned ? "true" : "false") << "\n";

    // ------------------ pickNextEntry ------------------
    bool assignToLeft = false;
    int nextEntry = pickNextEntry(leftNode, rightNode, leafEntries, assigned, assignToLeft);
    std::cout << "pickNextEntry index: " << nextEntry 
              << ", assignToLeft: " << (assignToLeft ? "true" : "false") << "\n";

    // ------------------ quadraticSplitEntries ------------------
    auto splitLeft = std::make_shared<RTreeNode>(true, 4);
    auto splitRight = std::make_shared<RTreeNode>(true, 4);
    quadraticSplitEntries(leafEntries, splitLeft, splitRight, 4);
    std::cout << "quadraticSplitEntries done. SplitLeft size: " << splitLeft->getLeafEntries().size()
              << ", SplitRight size: " << splitRight->getLeafEntries().size() << "\n";

    // ------------------ splitLeaf ------------------
    auto leafNode = std::make_shared<RTreeNode>(true, 4);
    for (const auto& entry : leafEntries) leafNode->insertLeaf(entry.first, entry.second);
    auto [leftLeaf, rightLeaf] = leafNode->splitLeaf();
    std::cout << "splitLeaf done. LeftLeaf size: " << leftLeaf->getLeafEntries().size()
              << ", RightLeaf size: " << rightLeaf->getLeafEntries().size() << "\n";

    // ------------------ splitInternal ------------------
    auto internalNode = std::make_shared<RTreeNode>(false, 4);
    for (int i = 0; i < 3; ++i) {
        auto child = std::make_shared<RTreeNode>(true, 4);
        child->insertLeaf(leafEntries[i].first, leafEntries[i].second);
        internalNode->insertChild(child->getMBR(), child);
    }
    auto [leftInternal, rightInternal] = internalNode->splitInternal();
    std::cout << "splitInternal done. LeftInternal children: " << leftInternal->getChildEntries().size()
              << ", RightInternal children: " << rightInternal->getChildEntries().size() << "\n";

    std::cout << "=== All Tests Completed ===\n";
    return 0;
}
