#include "../include/RTree.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <queue>
#include <arrow/io/file.h>
#include <arrow/table.h>
#include <arrow/array.h>
#include <arrow/record_batch.h>
#include <parquet/arrow/reader.h>

// Utility namespace for timestamp parsing
namespace timeUtil {
    int parseTimestampToSeconds(const std::string& timestamp);
}

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

    auto [splitLeft, splitRight] = root->insertRecursive(traj);

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
        insert(traj);
    }
    return true;
}

// ---------------- Queries ----------------
std::vector<Trajectory> RTree::rangeQuery(const BoundingBox3D& queryBox) const {
    std::vector<Trajectory> results;
    if (root) root->rangeQuery(queryBox, results);
    return results;
}

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
        out << j.dump(2);
    } catch (const std::exception& ex) {
        std::cerr << "Export error: " << ex.what() << std::endl;
    }
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

    auto sortByAxis = [](std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& v, int axis) {
        std::sort(v.begin(), v.end(), [axis](const auto& a, const auto& b) {
            if (axis == 0) return a.first.getMinX() < b.first.getMinX();
            if (axis == 1) return a.first.getMinY() < b.first.getMinY();
            return a.first.getMinT() < b.first.getMinT(); // <-- now int64_t comparison
        });
    };

    std::function<std::shared_ptr<RTreeNode>(std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>&, int)> buildSTR;
    buildSTR = [&](std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>>& entries, int axis) -> std::shared_ptr<RTreeNode> {
        if (entries.size() <= static_cast<size_t>(maxEntries)) {
            auto leaf = std::make_shared<RTreeNode>(true, maxEntries);
            for (auto& [bbox, trajPtr] : entries)
                leaf->insertLeaf(bbox, trajPtr);
            return leaf;
        }

        sortByAxis(entries, axis % 3);

        size_t sliceCount = std::ceil(std::sqrt(entries.size() / static_cast<double>(maxEntries)));
        size_t sliceSize = std::ceil(entries.size() / static_cast<double>(sliceCount));

        std::vector<std::shared_ptr<RTreeNode>> childNodes;
        for (size_t i = 0; i < entries.size(); i += sliceSize) {
            size_t end = std::min(i + sliceSize, entries.size());
            std::vector<std::pair<BoundingBox3D, std::shared_ptr<Trajectory>>> slice(
                entries.begin() + i, entries.begin() + end);

            childNodes.push_back(buildSTR(slice, axis + 1));
        }

        auto parent = std::make_shared<RTreeNode>(false, maxEntries);
        for (auto& child : childNodes)
            parent->insertChild(child->getMBR(), child);

        return parent;
    };

    root = buildSTR(entries, 0);
    root->recomputeMBRs(); 
}

// ---------------- Helper for faster queries ----------------
std::vector<TrajectorySummary> RTree::computeSummaries(const std::vector<Trajectory>& trajectories) {
    std::vector<TrajectorySummary> summaries;

    for (const auto& traj : trajectories) {
        BoundingBox3D bbox = traj.getBoundingBox();

        float sumX = 0.0f, sumY = 0.0f;
        int64_t sumT = 0; // <-- store sumT as int64_t

        for (const auto& pt : traj.getPoints()) {
            sumX += pt.getX();
            sumY += pt.getY();
            sumT += pt.getT(); // <-- directly use int64_t
        }

        size_t n = traj.getPoints().size();
        float centroidX = n > 0 ? sumX / n : 0.0f;
        float centroidY = n > 0 ? sumY / n : 0.0f;
        float centroidT = n > 0 ? static_cast<float>(sumT) / n : 0.0f; // convert to float for summary

        summaries.push_back({traj.getId(), bbox, centroidX, centroidY, centroidT, std::make_shared<Trajectory>(traj)});
    }

    return summaries;
}

// ---------------- Load from Parquet ----------------
std::vector<Trajectory> RTree::loadFromParquet(const std::string& filepath) {
    std::unordered_map<std::string, Trajectory> traj_map;

    std::shared_ptr<arrow::io::ReadableFile> infile;
    PARQUET_ASSIGN_OR_THROW(infile, arrow::io::ReadableFile::Open(filepath));

    std::unique_ptr<parquet::arrow::FileReader> parquet_reader;
    PARQUET_ASSIGN_OR_THROW(
        parquet_reader,
        parquet::arrow::OpenFile(
            std::static_pointer_cast<arrow::io::RandomAccessFile>(infile),
            arrow::default_memory_pool()
        )
    );

    auto batch_reader_res = parquet_reader->GetRecordBatchReader();
    PARQUET_THROW_NOT_OK(batch_reader_res.status());
    std::unique_ptr<arrow::RecordBatchReader> batch_reader = std::move(*batch_reader_res);

    std::shared_ptr<arrow::RecordBatch> batch;
    while (true) {
        PARQUET_THROW_NOT_OK(batch_reader->ReadNext(&batch));
        if (!batch) break;

        auto vehicle_col_array = batch->GetColumnByName("vehicle_id");
        auto trip_col_array    = batch->GetColumnByName("trip_id");
        auto x_col_array       = batch->GetColumnByName("x");
        auto y_col_array       = batch->GetColumnByName("y");
        auto t_col_array       = batch->GetColumnByName("t");

        if (!vehicle_col_array || !trip_col_array || !x_col_array || !y_col_array || !t_col_array)
            throw std::runtime_error("Missing columns in batch");

        int64_t num_rows = batch->num_rows();
        for (int64_t i = 0; i < num_rows; ++i) {
            if (!vehicle_col_array->IsValid(i) || !trip_col_array->IsValid(i) ||
                !x_col_array->IsValid(i) || !y_col_array->IsValid(i) || !t_col_array->IsValid(i))
                continue;

            int vehicle_id = std::static_pointer_cast<arrow::Int32Array>(vehicle_col_array)->Value(i);
            int trip_id    = std::static_pointer_cast<arrow::Int32Array>(trip_col_array)->Value(i);
            float x        = std::static_pointer_cast<arrow::FloatArray>(x_col_array)->Value(i);
            float y        = std::static_pointer_cast<arrow::FloatArray>(y_col_array)->Value(i);
            int64_t t      = std::static_pointer_cast<arrow::Int64Array>(t_col_array)->Value(i); // <-- int64_t
            //std::cout<< "Debug: Loaded point - vehicle_id: " << vehicle_id << ", trip_id: " << trip_id << ", x: " << x << ", y: " << y << ", t: " << t << std::endl;

            std::string traj_id = std::to_string(vehicle_id) + "_" + std::to_string(trip_id);

            auto it = traj_map.find(traj_id);
            if (it == traj_map.end()) {
                traj_map.emplace(traj_id, Trajectory(traj_id));
                it = traj_map.find(traj_id);
            }

            it->second.addPoint(Point3D(x, y, t)); // <-- t as int64_t
        }
    }

    std::vector<Trajectory> trajectories;
    trajectories.reserve(traj_map.size());
    for (auto& [_, traj] : traj_map) {
        trajectories.push_back(std::move(traj));
    }

    return trajectories;
}




/*
std::vector<Trajectory> RTree::loadFromJSON(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) throw std::runtime_error("Cannot open file: " + filepath);

    json j;
    in >> j;
    if (!j.is_array()) throw std::runtime_error("Expected JSON array of trajectories");

    std::vector<Trajectory> trajectories;
    trajectories.reserve(j.size());

    for (size_t idx = 0; idx < j.size(); ++idx) {
        const auto& traj_json = j[idx];

        // --- Extract and clean ID ---
        std::string rawId = traj_json.contains("id") ? traj_json["id"].get<std::string>() : "unknown";
        if (!rawId.empty() && rawId.front() == '"' && rawId.back() == '"') {
            rawId = rawId.substr(1, rawId.size() - 2);
        }

        Trajectory traj(rawId);

        // --- Add points ---
        if (traj_json.contains("points")) {
            const auto& pts = traj_json["points"];
            traj.reservePoints(pts.size());
            for (const auto& pt : pts) {
                traj.addPoint(Point3D(
                    std::stof(pt["x"].get<std::string>()),
                    std::stof(pt["y"].get<std::string>()),
                    pt.value("t", "")
                ));
            }
        }

        trajectories.push_back(std::move(traj));
    }

    std::cout << "Total trajectories loaded: " << trajectories.size() << "\n";
    return trajectories;
}*/