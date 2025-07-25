#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "../include/point3D.h"
#include "../include/bbox3D.h"
#include "../include/RTreeNode.h"
#include "../include/trajectory.h"
#include "../include/RTree.h"

/*
int main() {
    std::ifstream file("../../data/trajectory_data.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file.\n";
        return 1;
    }

    std::vector<Point3D> points;
    std::string line;

    // Skip header
    std::getline(file, line);

    std::cout << "\n=== Loading First 20 Points ===\n";
    int lineCount = 0;
    while (std::getline(file, line) && lineCount < 20) {
        std::istringstream ss(line);
        std::string token;

        float x = 0.0f, y = 0.0f;
        int32_t t = 0;
        int fieldIndex = 0;

        while (std::getline(ss, token, ',')) {
            switch (fieldIndex) {
                case 2: x = std::stof(token); break;
                case 3: y = std::stof(token); break;
                case 4: t = std::stoi(token); break;
            }
            fieldIndex++;
        }

        Point3D pt(x, y, t);
        pt.print();
        pt.validate();
        points.push_back(pt);
        lineCount++;
    }
    file.close();

    // === RTreeNode Insertion ===
    std::cout << "\n=== Inserting Points into RTreeNode ===\n";
    auto root = std::make_shared<RTreeNode>(true); // Start with a leaf node

    for (const auto& pt : points) {
        root->insert(pt);
    }

    std::cout << "Inserted " << points.size() << " points into R-tree.\n";

    // === RTree Range Query ===
    std::cout << "\n=== RTreeNode Range Query Test ===\n";
    BoundingBox3D queryBox;
    for (int i = 5; i < 15 && i < (int)points.size(); ++i) {
        queryBox.expandToInclude(points[i]);
    }

    std::vector<Point3D> queryResults;
    root->rangeQuery(queryBox, queryResults);

    std::cout << "Query Box:\n";
    std::cout << "Min: (" << queryBox.getMinX() << ", " << queryBox.getMinY() << ", " << queryBox.getMinT() << ")\n";
    std::cout << "Max: (" << queryBox.getMaxX() << ", " << queryBox.getMaxY() << ", " << queryBox.getMaxT() << ")\n";
    std::cout << "Found " << queryResults.size() << " matching points:\n";
    for (const auto& pt : queryResults) {
        pt.print();
    }

    // === Out-of-Range Tests ===
    std::cout << "\n=== Out-of-Range Tests ===\n";
    Point3D invalid1(1.5f, 0.5f, 5000);
    Point3D invalid2(0.3f, 0.7f, -10);
    invalid1.print(); invalid1.validate();
    invalid2.print(); invalid2.validate();

    // === Copy and Move Tests ===
    std::cout << "\n=== Copy and Move Tests ===\n";
    Point3D original = points[0];
    Point3D copyConstructed(original);
    Point3D copyAssigned;
    copyAssigned = original;
    Point3D moved(std::move(original));
    Point3D moveAssigned;
    moveAssigned = std::move(copyConstructed);
    copyAssigned.print();
    moveAssigned.print();

    // === Equality Tests ===
    std::cout << "\n=== Equality Tests ===\n";
    Point3D pt1(0.5f, 0.5f, 1000);
    Point3D pt2(0.5f, 0.5f, 1000);
    Point3D pt3(0.6f, 0.5f, 1000);
    std::cout << "pt1 == pt2: " << (pt1 == pt2 ? "true" : "false") << "\n";
    std::cout << "pt1 == pt3: " << (pt1 == pt3 ? "true" : "false") << "\n";

    // === Bounding Boxes ===
    std::cout << "\n=== Constructing Bounding Boxes ===\n";
    BoundingBox3D bbox1;
    for (int i = 0; i < 10 && i < (int)points.size(); i++) {
        bbox1.expandToInclude(points[i]);
    }

    BoundingBox3D bbox2;
    for (int i = 10; i < 20 && i < (int)points.size(); i++) {
        bbox2.expandToInclude(points[i]);
    }

    std::cout << "Bounding Box 1 Min: (" << bbox1.getMinX() << ", " << bbox1.getMinY() << ", " << bbox1.getMinT() << ")\n";
    std::cout << "Bounding Box 1 Max: (" << bbox1.getMaxX() << ", " << bbox1.getMaxY() << ", " << bbox1.getMaxT() << ")\n";
    std::cout << "Bounding Box 2 Min: (" << bbox2.getMinX() << ", " << bbox2.getMinY() << ", " << bbox2.getMinT() << ")\n";
    std::cout << "Bounding Box 2 Max: (" << bbox2.getMaxX() << ", " << bbox2.getMaxY() << ", " << bbox2.getMaxT() << ")\n";

    // === Containment Tests ===
    std::cout << "\n=== Containment Tests ===\n";
    std::cout << "bbox1 contains pt1? " << (bbox1.contains(pt1) ? "Yes" : "No") << "\n";
    std::cout << "bbox2 contains pt1? " << (bbox2.contains(pt1) ? "Yes" : "No") << "\n";

    // === Intersection Tests ===
    std::cout << "\n=== Intersection Tests ===\n";
    std::cout << "bbox1 intersects bbox2? " << (bbox1.intersects(bbox2) ? "Yes" : "No") << "\n";

    // === JSON Serialization ===
    std::cout << "\n=== Serializing Points and Bounding Boxes to JSON Files ===\n";

    // Serialize points
    json points_json = json::array();
    for (const auto& pt : points) {
        points_json.push_back(pt.to_json());
    }
    std::ofstream pointsFile("points_output.json");
    if (pointsFile.is_open()) {
        pointsFile << points_json.dump(4);
        pointsFile.close();
        std::cout << "Saved points_output.json\n";
    } else {
        std::cerr << "Failed to write points_output.json\n";
    }

    // Serialize both bounding boxes
    json bboxes_json = json::array();
    bboxes_json.push_back(bbox1.to_json());
    bboxes_json.push_back(bbox2.to_json());

    std::ofstream bboxFile("bboxes_output.json");
    if (bboxFile.is_open()) {
        bboxFile << bboxes_json.dump(4);
        bboxFile.close();
        std::cout << "Saved bboxes_output.json\n";
    } else {
        std::cerr << "Failed to write bboxes_output.json\n";
    }


    // === Serialize RTree ===
    std::cout << "\n=== Serializing RTree ===\n";
    json rtreeJson = root->to_json();

    std::ofstream rtreeFile("rtree_output.json");
    if (rtreeFile.is_open()) {
        rtreeFile << rtreeJson.dump(4);
        rtreeFile.close();
        std::cout << "Saved rtree_output.json\n";
    } else {
    std::cerr << "Failed to write rtree_output.json\n";
    }


    return 0;
}
*/
#include "../include/point3D.h"
#include "../include/bbox3D.h"
#include "../include/trajectory.h"
#include "../include/RTree.h"
#include "../include/RTreeNode.h"
#include <iostream>

void testPoint3D() {
    Point3D pt(0.5, 0.5, 1234);
    pt.validate();
    pt.print();
}

void testBoundingBox() {
    BoundingBox3D box;
    Point3D pt1(0.1f, 0.2f, 1000);
    Point3D pt2(0.4f, 0.5f, 1100);
    box.expandToInclude(pt1);
    box.expandToInclude(pt2);
    std::cout << "Volume: " << box.volume() << std::endl;
}

void testTrajectoryAndTree() {
    std::vector<Point3D> pts = {
        Point3D(0.1f, 0.2f, 1000),
        Point3D(0.4f, 0.5f, 1100),
        Point3D(0.3f, 0.3f, 1200)
    };
    Trajectory traj(pts, "veh123");

    RTree tree(4);
    tree.insert(traj);

    BoundingBox3D queryBox(0.0f, 0.0f, 900, 0.5f, 0.5f, 1300);
    auto results = tree.rangeQuery(queryBox);
    std::cout << "Found " << results.size() << " trajectory(ies)\n";
}

int main() {
    testPoint3D();
    testBoundingBox();
    testTrajectoryAndTree();
    return 0;
}
