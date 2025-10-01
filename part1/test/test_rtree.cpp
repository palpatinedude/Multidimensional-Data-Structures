#include "../api/include/RTree.h"
#include "../api/include/trajectory.h"
#include "../api/include/bbox3D.h"
#include "../api/include/point3D.h"
#include "../api/include/RTreeNode.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

// ------------------ Helper Functions ------------------
int64_t iso8601ToSeconds(const std::string& isoTime) {
    std::tm tm = {};
    std::istringstream ss(isoTime);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return static_cast<int64_t>(std::mktime(&tm));
}

void printTrajectories(const std::vector<Trajectory>& trajs) {
    for (const auto& t : trajs) {
        std::cout << "Trajectory ID: " << t.getId() 
                  << ", Points: " << t.getPoints().size() << "\n";
    }
}

// ------------------ Basic Insert Test ------------------
void testRTreeBasicInsert() {
    std::cout << "\n=== testRTreeBasicInsert ===\n";
    RTree tree(2);

    Trajectory traj1("traj1");
    traj1.addPoint(Point3D(0, 0, 1000));
    traj1.addPoint(Point3D(1, 1, 1010));

    Trajectory traj2("traj2");
    traj2.addPoint(Point3D(2, 2, 1020));
    traj2.addPoint(Point3D(3, 3, 1030));

    Trajectory traj3("traj3");
    traj3.addPoint(Point3D(-1, -1, 950));
    traj3.addPoint(Point3D(0, 0, 960));

    tree.insert(traj1);
    tree.insert(traj2);
    tree.insert(traj3); 
    
    auto root = tree.getRoot();
    assert(root != nullptr);
    std::cout << "Root is leaf? " << root->isLeafNode() << "\n";
    std::cout << "Tree height: " << tree.getHeight() << "\n";
    std::cout << "Total entries: " << tree.getTotalEntries() << "\n";

   // BoundingBox3D queryBox(-2, -2, 900, 4, 4, 1040);
   // auto results = tree.rangeQuery(queryBox);
   // std::cout << "Range query returned " << results.size() << " trajectories\n";
   // assert(results.size() == 3);
}

// ------------------ Update and Remove Test ------------------
void testRTreeUpdateRemove() {
    std::cout << "\n=== testRTreeUpdateRemove ===\n";
    RTree tree(2);

    Trajectory traj("trajA");
    traj.addPoint(Point3D(0, 0, 1000));
    traj.addPoint(Point3D(1, 1, 1010));

    tree.insert(traj);

    traj.addPoint(Point3D(2, 2, 1020));
    bool updated = tree.update(traj);
    std::cout << "Update success? " << updated << "\n";
    assert(updated);

    bool removed = tree.remove(traj.getId());
    std::cout << "Remove success? " << removed << "\n";
    assert(removed);
    std::cout << "Total Entries :"<< tree.getTotalEntries() << "\n";
}

// ------------------ kNN and Similarity Test ------------------
void testRTreeKNNAndSimilarity() {
    std::cout << "\n=== testRTreeKNNAndSimilarity ===\n";
    RTree tree(2);

    Trajectory t1("t1"); t1.addPoint(Point3D(0, 0, 1000));
    Trajectory t2("t2"); t2.addPoint(Point3D(5, 5, 1000));
    Trajectory t3("t3"); t3.addPoint(Point3D(10, 10, 1000));

    tree.insert(t1); tree.insert(t2); tree.insert(t3);

    auto knn = tree.kNearestNeighbors(t1, 2);
    std::cout << "kNN returned " << knn.size() << " trajectories\n";
    printTrajectories(knn);
    assert(knn.size() == 2);

    auto similar = tree.findSimilar(t1, 10.0f);
    std::cout << "Similarity search returned " << similar.size() << " trajectories\n";
    printTrajectories(similar);
    assert(!similar.empty());
}

// ------------------ Bulk Load Synthetic Test ------------------
void testRTreeBulkLoadSynthetic() {
    std::cout << "\n=== testRTreeBulkLoadSynthetic ===\n";
    std::vector<Trajectory> trajs;
    for (int i = 0; i < 10; ++i) {
        Trajectory t("bulk_" + std::to_string(i));
        for (int j = 0; j < 5; ++j)
            t.addPoint(Point3D(i+j, i-j, 1000+j*10));
        trajs.push_back(t);
    }

    RTree tree(3);
    tree.bulkLoad(trajs);
    std::cout << "Tree height after synthetic bulk load: " << tree.getHeight() << "\n";
    std::cout << "Total entries: " << tree.getTotalEntries() << "\n";

    auto summaries = tree.computeSummaries(trajs);
    for (const auto& s : summaries)
        std::cout << "Summary: id=" << s.id 
                  << ", centroid=(" << s.centroidX << "," 
                  << s.centroidY << "," << s.centroidT << ")\n";
}

// ------------------ Bulk Load Real Parquet Test ------------------
void testRTreeBulkLoadParquet() {
    std::cout << "\n=== testRTreeBulkLoadParquet ===\n";
    std::string parquetDir = "../../preprocessing/trajectories_grouped.parquet";
    RTree tree(10);
    std::vector<Trajectory> trajectories;

    // Load all Parquet files in directory
    for (const auto& entry : fs::directory_iterator(parquetDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".parquet") {
            auto partial = RTree::loadFromParquet(entry.path().string());
            trajectories.insert(trajectories.end(), partial.begin(), partial.end());
        }
    }

    std::cout << "Loaded " << trajectories.size() << " trajectories from Parquet files\n";
    tree.bulkLoad(trajectories);

    std::cout << "Tree height after Parquet bulk load: " << tree.getHeight() << "\n";
    std::cout << "Total entries: " << tree.getTotalEntries() << "\n";

    if (!trajectories.empty()) {
        Trajectory tnew("new_test_traj");
        tnew.addPoint(Point3D(999, 999, 2000));
        tree.insert(tnew);
        std::cout << "Inserted new trajectory\n";

        Trajectory tmod = trajectories.front();
        tmod.addPoint(Point3D(888, 888, 3000));
        tree.update(tmod);
        std::cout << "Updated trajectory " << tmod.getId() << "\n";

        tree.remove(tmod.getId());
        std::cout << "Removed trajectory " << tmod.getId() << "\n";
    }

    BoundingBox3D queryBox(-1000, -1000, 1000, 1000, 1000, 2000);
    auto results = tree.rangeQuery(queryBox);
    std::cout << "Range query returned " << results.size() << " trajectories\n";
}

// ------------------ Stress Test ------------------
void testRTreeStress() {
    std::cout << "\n=== testRTreeStress ===\n";
    RTree tree(10);

    const int numTrajectories = 1000;
    const int pointsPerTraj = 20;

    for (int i = 0; i < numTrajectories; ++i) {
        Trajectory t("traj_" + std::to_string(i));

        for (int j = 0; j < pointsPerTraj; ++j) {
            // Generate valid coordinates
            float x = fmod(i + j * 0.1f, 360.0f) - 180.0f; // longitude [-180,180]
            float y = fmod(i - j * 0.1f, 180.0f) - 90.0f;   // latitude [-90,90]
            float time = 1.0f + j;                           // timestamp > 0

            t.addPoint(Point3D(x, y, time));
        }

        tree.insert(t);
    }

    std::cout << "Stress test: inserted " << numTrajectories << " trajectories\n";
    std::cout << "Tree height: " << tree.getHeight() << "\n";
    std::cout << "Total entries: " << tree.getTotalEntries() << "\n";

    BoundingBox3D queryBox(-100, -50, 1, 100, 50, 20);
    auto results = tree.rangeQuery(queryBox);
    std::cout << "Range query returned " << results.size() << " trajectories\n";
}

// ------------------ ISO 8601 Range Query Examples ------------------
void testRTreeISOQueries() {
    std::cout << "\n=== testRTreeISOQueries ===\n";
    RTree tree(10);

    std::string parquetDir = "../../preprocessing/trajectories_grouped.parquet";
    std::vector<Trajectory> trajectories;
    for (const auto& entry : fs::directory_iterator(parquetDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".parquet") {
            auto partial = RTree::loadFromParquet(entry.path().string());
            trajectories.insert(trajectories.end(), partial.begin(), partial.end());
        }
    }

    for (auto& t : trajectories) tree.insert(t);

    struct QueryRange { std::string startISO, endISO; };
    std::vector<QueryRange> queries = {
        {"2017-07-15T09:00:00Z","2017-09-20T17:00:00Z"},
        {"2017-12-03T14:30:00Z","2018-04-05T19:00:00Z"},
        {"2018-06-10T08:00:00Z","2018-10-15T12:30:00Z"},
        {"2019-02-20T13:15:00Z","2019-06-25T18:45:00Z"},
        {"2018-08-15T14:30:00Z","2025-02-03T09:45:00Z"}
    };

    for (size_t i = 0; i < queries.size(); ++i) {
        auto start = iso8601ToSeconds(queries[i].startISO);
        auto end = iso8601ToSeconds(queries[i].endISO);
        BoundingBox3D qBox(-1000,-1000,start,1000,1000,end); 
        auto res = tree.rangeQuery(qBox);
        std::cout << "ISO Query " << i+1 << " returned " << res.size() << " trajectories\n";
    }
}

// ------------------ Main ------------------
int main() {
   // testRTreeBasicInsert();
    testRTreeUpdateRemove();
  //  testRTreeKNNAndSimilarity();
  //  testRTreeBulkLoadSynthetic();
 //   testRTreeBulkLoadParquet();
  //  testRTreeStress();
   // testRTreeISOQueries();

    std::cout << "\n=== All RTree tests completed successfully ===\n";
    return 0;
}

