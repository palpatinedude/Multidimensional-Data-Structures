#include "../include/RTree.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <queue>

// ---------------- Constructor ----------------
RTree::RTree(int maxEntries)
    : maxEntries(maxEntries) {
    root = std::make_shared<RTreeNode>(true, maxEntries); // Root starts as a leaf
}

// ---------------- Insertion ----------------
void RTree::insert(const Trajectory& traj) {
    if (!root) {
        root = std::make_shared<RTreeNode>(true, maxEntries);
    }

    // Insert into root; may cause a split
    auto [splitLeft, splitRight] = root->insertRecursive(traj);

    // If split occurred, create a new root
    if (splitLeft && splitRight) {
        auto newRoot = std::make_shared<RTreeNode>(false, maxEntries);
        newRoot->insertChild(splitLeft->getMBR(), splitLeft);
        newRoot->insertChild(splitRight->getMBR(), splitRight);
        newRoot->updateMBR();
        root = newRoot;
    }
}

// ---------------- Deletion & Update ----------------
bool RTree::remove(const std::string& trajId) {
    return root ? root->deleteTrajectory(trajId) : false;
}

bool RTree::update(const Trajectory& traj) {
    if (!root) return false;
    if (!root->updateTrajectory(traj)) {
        insert(traj); // Reinsert if not found
    }
    return true;
}

// ---------------- Queries ----------------
std::vector<Trajectory> RTree::rangeQuery(const BoundingBox3D& queryBox) const {
    std::vector<Trajectory> results;
    if (root) root->rangeQuery(queryBox, results);
    return results;
}
/*
std::vector<Trajectory> RTree::kNearestNeighbors(const Trajectory& query, size_t k) const {
    return root ? root->kNearestNeighbors(query, k) : std::vector<Trajectory>{};
}*/

std::vector<Trajectory> RTree::kNearestNeighbors(const Trajectory& query, size_t k, float timeScale) const {
    return root ? root->kNearestNeighbors(query, k, timeScale) : std::vector<Trajectory>{};
}

std::vector<Trajectory> RTree::findSimilar(const Trajectory& query, float maxDistance) const {
    std::vector<Trajectory> results;
    if (root) root->findSimilar(query, maxDistance, results);
    return results;
}

std::vector<Trajectory> RTree::getAllLeafTrajectories() const {
    std::vector<Trajectory> results;
    if (!root) return results;

    std::queue<std::shared_ptr<RTreeNode>> q;
    q.push(root);

    while (!q.empty()) {
        auto node = q.front(); q.pop();
        if (node->isLeafNode()) {
            for (const auto& [_, trajPtr] : node->getLeafEntries()) {
                results.push_back(*trajPtr);
            }
        } else {
            for (const auto& [_, child] : node->getChildEntries()) {
                q.push(child);
            }
        }
    }
    return results;
}


// ---------------- Persistence ----------------
void RTree::exportToJSON(const std::string& filename) const {
    try {
        std::ofstream out(filename);
        if (!out) throw std::runtime_error("Failed to open file for writing");
        json j = root->to_json();
        out << j.dump(2); // Pretty print
    } catch (const std::exception& ex) {
        std::cerr << "Export error: " << ex.what() << std::endl;
    }
}

std::vector<Trajectory> RTree::loadFromJSON(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) throw std::runtime_error("Cannot open file: " + filepath);

    nlohmann::json j;
    in >> j;
    if (!j.is_array()) throw std::runtime_error("Expected JSON array of trajectories");

    std::vector<Trajectory> trajectories;
    trajectories.reserve(16000);

    auto strToFloat = [](const nlohmann::json& val, float def = 0.0f) -> float {
        try {
            return std::strtof(val.get<std::string>().c_str(), nullptr);
        } catch (...) {
            return def;
        }
    };

    for (const auto& traj_json : j) {
        Trajectory traj(traj_json.value("id", "unknown"));

        if (traj_json.contains("points")) {
            const auto& pts = traj_json["points"];
            traj.reservePoints(pts.size());
            for (const auto& pt : pts) {
                traj.addPoint(Point3D(
                    strToFloat(pt["x"]),
                    strToFloat(pt["y"]),
                    pt.value("t", "")
                ));
            }
        }
        trajectories.push_back(std::move(traj));
    }
    std::cout << "Total trajectories loaded: " << trajectories.size() << "\n";
    return trajectories;
}

// ---------------- Stats ----------------
size_t RTree::getTotalEntries() const {
    if (!root) return 0;
    std::queue<std::shared_ptr<RTreeNode>> q;
    q.push(root);
    size_t count = 0;

    while (!q.empty()) {
        auto node = q.front(); q.pop();
        if (node->isLeafNode())
            count += node->getLeafEntries().size();
        else
            for (const auto& [_, child] : node->getChildEntries())
                q.push(child);
    }
    return count;
}

int RTree::getHeight() const {
    if (!root) return 0;
    std::queue<std::shared_ptr<RTreeNode>> q;
    q.push(root);
    int height = 0;

    while (!q.empty()) {
        int levelSize = q.size();
        for (int i = 0; i < levelSize; ++i) {
            auto node = q.front(); q.pop();
            if (!node->isLeafNode())
                for (const auto& [_, child] : node->getChildEntries())
                    q.push(child);
        }
        height++;
    }
    return height;
}

void RTree::printStatistics() const {
    std::cout << "========= RTree Statistics =========\n";
    std::cout << "Total entries: " << getTotalEntries() << "\n";
    std::cout << "Tree height: " << getHeight() << "\n";
    std::cout << "Max entries per node: " << maxEntries << "\n";
}


// ---------------- Bulk Load ----------------
void RTree::bulkLoad(std::vector<Trajectory>& trajectories) {
    // If there are no trajectories, the tree is empty
    if (trajectories.empty()) {
        root = nullptr;
        return;
    }

    // Step 1: Prepare entries for bulk loading
    // Each entry is a pair of bounding box and trajectory pointer
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> entries;
    entries.reserve(trajectories.size());
    for (Trajectory& traj : trajectories) {
        // Use shared_ptr so nodes can share trajectory data safely
        auto trajPtr = std::make_shared<Trajectory>(std::move(traj));
        // Store bounding box and trajectory together
        entries.emplace_back(trajPtr->getBoundingBox(), trajPtr);
    }

    // Helper function: sort entries along a given axis (0=X, 1=Y, 2=T)
    auto sortByAxis = [](std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& v, int axis) {
        std::sort(v.begin(), v.end(), [axis](const auto& a, const auto& b) {
            if (axis == 0) return a.first.getMinX() < b.first.getMinX(); // sort by X
            if (axis == 1) return a.first.getMinY() < b.first.getMinY(); // sort by Y
            return a.first.getMinT() < b.first.getMinT();                 // sort by T
        });
    };

    // Recursive function to build STR tree from entries
    std::function<std::shared_ptr<RTreeNode>(std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>&, int)> buildSTR;
    buildSTR = [&](std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& entries, int axis) -> std::shared_ptr<RTreeNode> {
        // Base case: if number of entries is small enough, create a leaf node
        if (entries.size() <= static_cast<size_t>(maxEntries)) {
            auto leaf = std::make_shared<RTreeNode>(true, maxEntries); // leaf = true
            for (auto& [bbox, trajPtr] : entries)
                leaf->insertLeaf(bbox, trajPtr); // insert trajectory into leaf
            return leaf;
        }

        // Step 2: Sort entries along the current axis
        sortByAxis(entries, axis % 3);

        // Step 3: Slice entries into groups for next level
        size_t sliceCount = std::ceil(std::sqrt(entries.size() / static_cast<double>(maxEntries)));
        size_t sliceSize = std::ceil(entries.size() / static_cast<double>(sliceCount));

        std::vector<std::shared_ptr<RTreeNode>> childNodes;
        for (size_t i = 0; i < entries.size(); i += sliceSize) {
            size_t end = std::min(i + sliceSize, entries.size());
            // Take a slice of entries
            std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> slice(
                entries.begin() + i, entries.begin() + end);

            // Recursive call to build child node along the next axis
            childNodes.push_back(buildSTR(slice, axis + 1));
        }

        // Step 4: Create internal node and attach child nodes
        auto parent = std::make_shared<RTreeNode>(false, maxEntries); // leaf = false
        for (auto& child : childNodes)
            parent->insertChild(child->getMBR(), child); // insert each child's MBR

        return parent; // return internal node
    };

    // Start building the tree from axis 0 (X-axis)
    root = buildSTR(entries, 0);

    // Make sure all internal node MBRs are updated to include all children
    root->recomputeMBRs(); 
}





/*
bool RTree::update(const Trajectory& traj) {
    return root->updateTrajectory(traj);
}*/


/*
void RTree::bulkLoad(std::vector<Trajectory>& trajectories) {
    if (trajectories.empty()) {
        root = nullptr;
        return;
    }

    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> entries;
    entries.reserve(trajectories.size());

    for (Trajectory& traj : trajectories) {
        auto trajPtr = std::make_shared<Trajectory>(std::move(traj));
        entries.emplace_back(trajPtr->getBoundingBox(), trajPtr);
    }

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.first.getMinX() < b.first.getMinX();
    });

    // Step 3: Build leaf nodes
    std::vector<std::shared_ptr<RTreeNode>> currentLevel;
    for (size_t i = 0; i < entries.size(); i += maxEntries) {
        auto leaf = std::make_shared<RTreeNode>(true, maxEntries);
        size_t end = std::min(i + maxEntries, entries.size());
        for (size_t j = i; j < end; ++j) {
            leaf->insertLeaf(entries[j].first, entries[j].second);
        }
        currentLevel.push_back(leaf);
    }

    // Step 4: Build internal levels (with parent connection fix)
    while (currentLevel.size() > 1) {
        std::vector<std::shared_ptr<RTreeNode>> nextLevel;

        for (size_t i = 0; i < currentLevel.size(); i += maxEntries) {
            auto parent = std::make_shared<RTreeNode>(false, maxEntries);
            size_t end = std::min(i + maxEntries, currentLevel.size());
            for (size_t j = i; j < end; ++j) {
                auto& child = currentLevel[j];
                parent->insertChild(child->getMBR(), child);  
            }
            nextLevel.push_back(parent);
        }

        currentLevel = std::move(nextLevel);
    }

    root = currentLevel.front();
    root->recomputeMBRs();  // Ensures all internal MBRs are correct

} */



/*
std::vector<Trajectory> RTree::loadFromJSON(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) {
        throw std::runtime_error("Error opening JSON file: " + filepath);
    }

    json j;
    in >> j;

    std::vector<Trajectory> trajectories;

    if (!j.is_array()) {
        throw std::runtime_error("JSON format error: expected an array of trajectories");
    }

    // Helper: convert string to float safely
    auto strToFloat = [](const nlohmann::json& val, float defaultVal = 0.0f) -> float {
        try {
            return std::stof(val.get<std::string>());  // convert "12.30" -> 12.30f
        } catch (...) {
            return defaultVal;
        }
    };

    for (const auto& traj_json : j) {
        std::string traj_id = traj_json.value("id", "unknown");

        Trajectory traj(traj_id);

        // Points
        if (traj_json.contains("points") && traj_json["points"].is_array()) {
            for (const auto& pt : traj_json["points"]) {
                traj.addPoint(Point3D(
                    strToFloat(pt["x"]),
                    strToFloat(pt["y"]),
                    pt.value("t", "")
                ));
            }
        } else {
            std::cerr << "Warning: trajectory " << traj_id << " has no points\n";
        }

        // Bounding box
        if (traj_json.contains("bbox") && traj_json["bbox"].is_array() && traj_json["bbox"].size() == 6) {
            const auto& b = traj_json["bbox"];
            BoundingBox3D bbox_entry(
                strToFloat(b[0]), strToFloat(b[1]), b[2].get<std::string>(),
                strToFloat(b[3]), strToFloat(b[4]), b[5].get<std::string>()
            );
            traj.setCachedBBox(bbox_entry);
        } else {
            std::cerr << "Warning: trajectory " << traj_id << " missing or invalid bbox, recomputing...\n";
            // traj.recomputeBBox();
        }

        trajectories.push_back(std::move(traj));
    }

    std::cout << "Total trajectories loaded: " << trajectories.size() << std::endl;
    return trajectories;
}


std::vector<Trajectory> RTree::loadFromJSON(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) {
        throw std::runtime_error("Error opening JSON Lines file: " + filepath);
    }

    std::vector<Trajectory> trajectories;
    trajectories.reserve(15738);  // optional pre-allocation

    // Fast string-to-float converter
    auto strToFloat = [](const std::string& s, float defaultVal = 0.0f) -> float {
        try {
            return std::stof(s);
        } catch (...) {
            return defaultVal;
        }
    };

    std::string line;
    size_t line_number = 0;
    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty()) continue;

        try {
            json traj_json = json::parse(line);
            std::string traj_id = traj_json.value("id", "unknown");
            Trajectory traj(traj_id);

            // Points
            if (traj_json.contains("points") && traj_json["points"].is_array()) {
                auto& pts = traj_json["points"];
                traj.reservePoints(pts.size()); // pre-allocate inside Trajectory
                for (auto& pt : pts) {
                    traj.addPoint(Point3D(
                        strToFloat(pt["x"].get<std::string>()),
                        strToFloat(pt["y"].get<std::string>()),
                        pt.value("t", "")
                    ));
                }
            } else {
                std::cerr << "Warning: trajectory " << traj_id << " has no points\n";
            }

            // Bounding box
            if (traj_json.contains("bbox") && traj_json["bbox"].is_array() && traj_json["bbox"].size() == 6) {
                const auto& b = traj_json["bbox"];
                BoundingBox3D bbox_entry(
                    strToFloat(b[0].get<std::string>()),
                    strToFloat(b[1].get<std::string>()),
                    b[2].get<std::string>(),
                    strToFloat(b[3].get<std::string>()),
                    strToFloat(b[4].get<std::string>()),
                    b[5].get<std::string>()
                );
                traj.setCachedBBox(bbox_entry);
            } else {
                std::cerr << "Warning: trajectory " << traj_id << " missing or invalid bbox, recomputing...\n";
                // traj.recomputeBBox();
            }

            trajectories.push_back(std::move(traj)); // move instead of copy
        } catch (const std::exception& e) {
            std::cerr << "Error parsing line " << line_number << ": " << e.what() << "\n";
        }
    }

    std::cout << "Total trajectories loaded: " << trajectories.size() << std::endl;
    return trajectories;
}*/



/*
std::vector<Trajectory> RTree::loadFromJSON(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) {
        throw std::runtime_error("Error opening JSON file: " + filepath);
    }

    json j;
    in >> j;

    std::vector<Trajectory> trajectories;

    // Recursive lambda
    auto parseNode = [&](auto&& self, const json& node) -> void {
        bool isLeaf = node.value("leaf", false);
        size_t count = isLeaf ? node.value("entries", json::array()).size()
                              : node.value("children", json::array()).size();

        std::cout << (isLeaf ? "Leaf node" : "Internal node")
                  << " with " << count << " elements\n";

        if (isLeaf) {
            for (const auto& entry : node["entries"]) {
                if (!entry.contains("trajectory") || !entry.contains("box")) continue;

                const auto& traj_json = entry["trajectory"];
                const auto& box_json  = entry["box"];

                std::string traj_id = traj_json.value("id", "unknown");
                std::cout << "  Loading trajectory " << traj_id << "\n";

                Trajectory traj(traj_id);
                for (const auto& pt : traj_json.value("points", json::array())) {
                    traj.addPoint(Point3D(pt.value("x", 0.0f), pt.value("y", 0.0f), pt.value("t", "")));
                }

                BoundingBox3D bbox_entry(
                    box_json.value("minX", 0.0f),
                    box_json.value("minY", 0.0f),
                    box_json.value("minT", ""),
                    box_json.value("maxX", 0.0f),
                    box_json.value("maxY", 0.0f),
                    box_json.value("maxT", "")
                );
                traj.setCachedBBox(bbox_entry);

                trajectories.push_back(std::move(traj));
            }
        } else if (node.contains("children")) {
            for (const auto& child : node["children"]) {
                self(self, child);
            }
        } else {
            std::cerr << "  WARNING: Internal node has no 'children' field\n";
        }
    };

    parseNode(parseNode, j);

    std::cout << "Total trajectories loaded: " << trajectories.size() << std::endl;
    return trajectories;
}
*/


/*
void RTree::exportToJSON(const std::string& filename) const {
    try {
        std::ofstream out(filename);
        if (!out) throw std::runtime_error("Failed to open file for writing");

        json trajArray = json::array();

        auto allTrajs = getAllLeafTrajectories(); // implement to collect all leaf trajectories
        if (allTrajs.empty()) {
            std::cerr << "No trajectories to export." << std::endl;
            return;
        }
        std::cout << "Exporting " << allTrajs.size() << " trajectories to " << filename << std::endl;

        for (const auto& traj : allTrajs) {
            trajArray.push_back(traj.to_json()); // assuming Trajectory::to_json() matches loader format
        }

        out << trajArray.dump(2);
    } catch (const std::exception& ex) {
        std::cerr << "Export error: " << ex.what() << std::endl;
    }
}*/


/*
void RTree::insert(const Trajectory& traj) {
    if (!root) {
    root = std::make_shared<RTreeNode>(true, maxEntries);
    }
    auto splitNode = root->insertRecursive(traj);
    if (splitNode && splitNode.get() != root.get()) {
        auto newRoot = std::make_shared<RTreeNode>(false, maxEntries);
        newRoot->insert(root->getMBR(), root);
        newRoot->insert(splitNode->getMBR(), splitNode);
        root = newRoot;
    }
}*/

/*
void RTree::insert(const Trajectory& traj) {
    if (!root) {
        root = std::make_shared<RTreeNode>(true, maxEntries);
    }

    auto splitNode = root->insertRecursive(traj);

    // Only create a new root if the split returned a valid node
    if (splitNode) {
        auto newRoot = std::make_shared<RTreeNode>(false, maxEntries);

        if (!root->isEmpty()) {
            newRoot->insert(root->getMBR(), root);  // sets parent automatically
        }

        if (!splitNode->isEmpty()) {
            newRoot->insert(splitNode->getMBR(), splitNode);  // sets parent automatically
        }

        newRoot->updateMBR();
        root = newRoot;
    }
}*/


/*
void RTree::bulkLoad(const std::vector<Trajectory>& trajectories) {
    // Sort by space-filling curve (optional, better performance)
    std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> entries;
    for (const auto& traj : trajectories) {
        entries.emplace_back(traj.computeBoundingBox(), std::make_shared<Trajectory>(traj));
    }

    // Sort entries by minX for spatial locality (better than random)
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.first.getMinX() < b.first.getMinX();
    });

    // Bottom-up build
    std::vector<std::shared_ptr<RTreeNode>> leaves;
    for (size_t i = 0; i < entries.size(); i += maxEntries) {
        auto node = std::make_shared<RTreeNode>(true, maxEntries);
        size_t end = std::min(i + maxEntries, entries.size());
        for (size_t j = i; j < end; ++j) {
            node->insertLeafEntry(entries[j].first, entries[j].second);
        }
        leaves.push_back(node);
    }

    // Build upper levels
    while (leaves.size() > 1) {
        std::vector<std::shared_ptr<RTreeNode>> parents;
        for (size_t i = 0; i < leaves.size(); i += maxEntries) {
            auto parent = std::make_shared<RTreeNode>(false, maxEntries);
            size_t end = std::min(i + maxEntries, leaves.size());
            for (size_t j = i; j < end; ++j) {
                parent->insert(leaves[j]->getMBR(), leaves[j]);
            }
            parents.push_back(parent);
        }
        leaves = parents;
    }

    root = leaves.front();
}
*/