/*
 * splitHelpers.inl
 *
 * This header-only helper file provides functions to perform the "quadratic split"
 * operation on RTreeNode entries when a node overflows. These functions are:
 *
 * - pickSeeds: selects the two most wasteful entries to start a split.
 * - assignEntryToNode: adds an entry to a node, automatically handling leaf vs internal nodes.
 * - assignForcedEntries: ensures minimum fill is satisfied by assigning remaining entries to a node.
 * - pickNextEntry: chooses the next entry to assign based on MBR enlargement difference.
 * - quadraticSplitEntries: main function to split a set of entries between two nodes using the quadratic split algorithm.
 *
 * All functions are template-based to work with both leaf entries 
 * (BoundingBox3D + Trajectory) and internal entries (BoundingBox3D + RTreeNode).
 *
 * These functions are intended to be used only by RTreeNode internal operations and
 * assume that nodes are properly initialized and managed via shared_ptr.
 */

#ifndef SPLITHELPERS_INL
#define SPLITHELPERS_INL

#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include "RTreeNode.h"

// -------------------- pickSeeds --------------------
namespace splitHelpers {

// Pick the two most wasteful seeds for quadratic split
template <typename EntryType>
static std::pair<int, int> pickSeeds(const std::vector<EntryType>& entries) {
    int total = entries.size();
    int seed1 = -1, seed2 = -1;
    float worstWaste = -1.0f;

    // Compare each pair and pick the pair that wastes most volume
    for (int i = 0; i < total; ++i) {
        for (int j = i + 1; j < total; ++j) {
            BoundingBox3D combined = entries[i].first;
            combined.expandToInclude(entries[j].first);
            float waste = combined.volume() - entries[i].first.volume() - entries[j].first.volume();
            if (waste > worstWaste) {
                worstWaste = waste;
                seed1 = i;
                seed2 = j;
            }
        }
    }
    return {seed1, seed2};
}

// Assign an entry to the given node (leaf or internal) using compile-time type check
template <typename EntryType>
static void assignEntryToNode(std::shared_ptr<RTreeNode> node, const EntryType& entry) {
    if constexpr (std::is_same_v<EntryType, std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>) {
        node->insertLeaf(entry.first, entry.second);   // Leaf entry
    } else if constexpr (std::is_same_v<EntryType, std::pair<BoundingBox3D, std::shared_ptr<RTreeNode>>>) {
        node->insertChild(entry.first, entry.second);  // Internal node entry
    }
}

// Force-assign entries to nodes if one node needs entries to satisfy minimum fill
template <typename EntryType>
static bool assignForcedEntries(std::shared_ptr<RTreeNode> left,
                               std::shared_ptr<RTreeNode> right,
                               const std::vector<EntryType>& entries,
                               std::vector<bool>& assigned,
                               int minFill) 
{
    int total = entries.size();
    int leftSize  = left->isLeafNode() ? left->getLeafEntries().size() : left->getChildEntries().size();
    int rightSize = right->isLeafNode() ? right->getLeafEntries().size() : right->getChildEntries().size();
    int remaining = total - std::count(assigned.begin(), assigned.end(), true);

    // Assign all remaining entries to left if left cannot reach minFill otherwise
    if (leftSize + remaining <= minFill) {
        for (int i = 0; i < total; ++i) {
            if (!assigned[i]) {
                assignEntryToNode(left, entries[i]);
                assigned[i] = true;
            }
        }
        return true;
    }

    // Assign all remaining entries to right if right cannot reach minFill otherwise
    if (rightSize + remaining <= minFill) {
        for (int i = 0; i < total; ++i) {
            if (!assigned[i]) {
                assignEntryToNode(right, entries[i]);
                assigned[i] = true;
            }
        }
        return true;
    }

    return false;
}

// Pick the next entry to assign based on largest MBR enlargement difference
template <typename EntryType>
static int pickNextEntry(std::shared_ptr<RTreeNode> left,
                         std::shared_ptr<RTreeNode> right,
                         const std::vector<EntryType>& entries,
                         const std::vector<bool>& assigned,
                         bool& assignToLeft) 
{
    int total = entries.size();
    int next = -1;
    float maxDiff = -1.0f;

    for (int i = 0; i < total; ++i) {
        if (!assigned[i]) {
            float enlargeLeft  = left->enlargement(left->getMBR(), entries[i].first);
            float enlargeRight = right->enlargement(right->getMBR(), entries[i].first);
            float diff = std::fabs(enlargeLeft - enlargeRight);

            if (diff > maxDiff) {
                maxDiff = diff;
                next = i;
                // Tie-breaking: assign to the node with smaller enlargement or smaller MBR volume
                assignToLeft = (enlargeLeft < enlargeRight) ||
                               (enlargeLeft == enlargeRight && left->getMBR().volume() <= right->getMBR().volume());
            }
        }
    }

    return next;
}

// Perform quadratic split of entries into two nodes
template <typename EntryType>
static void quadraticSplitEntries(const std::vector<EntryType>& entries, 
                                  std::shared_ptr<RTreeNode> leftNode,
                                  std::shared_ptr<RTreeNode> rightNode,
                                  int maxEntries) 
{
    int minFill = (maxEntries + 1) / 2;

    // Step 1: Pick seeds
    auto [seed1, seed2] = pickSeeds(entries);
    assignEntryToNode(leftNode, entries[seed1]);
    assignEntryToNode(rightNode, entries[seed2]);

    // Track assigned entries
    std::vector<bool> assigned(entries.size(), false);
    assigned[seed1] = assigned[seed2] = true;

    // Step 2: Distribute remaining entries
    while (static_cast<size_t>(std::count(assigned.begin(), assigned.end(), true)) < entries.size()) {
        // Force assignment if minFill requires
        if (assignForcedEntries(leftNode, rightNode, entries, assigned, minFill)) break;

        // Pick next best entry to assign
        bool assignToLeft = false;
        int next = pickNextEntry(leftNode, rightNode, entries, assigned, assignToLeft);
        assignEntryToNode(assignToLeft ? leftNode : rightNode, entries[next]);
        assigned[next] = true;
    }

    // Step 3: Update MBRs
    leftNode->updateMBR();
    rightNode->updateMBR();
}


} // namespace splitHelpers

#endif // SPLITHELPERS_INL
