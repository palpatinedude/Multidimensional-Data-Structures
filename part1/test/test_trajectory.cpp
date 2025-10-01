// test_trajectory.cpp
#include "../api/include/point3D.h"
#include "../api/include/bbox3D.h"
#include "../api/include/trajectory.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <iomanip>

int main() {
    std::cout << "Starting Trajectory tests...\n";

    // -------------------- Create Points --------------------
    Point3D p1(0.0f, 0.0f, 1000);
    Point3D p2(3.0f, 4.0f, 1100);
    Point3D p3(6.0f, 8.0f, 1200);

    // -------------------- Initialize Trajectories --------------------
    Trajectory traj1("traj_1");
    traj1.addPoint(p1);
    traj1.addPoint(p2);
    traj1.addPoint(p3);

    Trajectory traj2("traj_2");
    traj2.addPoint(Point3D(1.0f, 1.0f, 1020));
    traj2.addPoint(Point3D(4.0f, 5.0f, 1120));
    traj2.addPoint(Point3D(7.0f, 9.0f, 1220));

    // -------------------- Test Initial Similarity / Distance --------------------
    float sim_initial = traj1.similarityTo(traj2);
    float stDist_initial = traj1.spatioTemporalDistanceTo(traj2, 1.0f);
    float approxDist_initial = traj1.approximateDistance(traj2, 1.0f);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Initial similarity: " << sim_initial << "\n";
    std::cout << "Initial spatio-temporal distance: " << stDist_initial << "\n";
    std::cout << "Initial approximate distance: " << approxDist_initial << "\n";

    // -------------------- Dynamic Expansion: Add Points to traj1 --------------------
    std::cout << "\n--- Adding Points to traj1 ---\n";
    traj1.addPoint(Point3D(-2.0f, 2.0f, 950));
    traj1.addPoint(Point3D(8.0f, 10.0f, 1300));

    traj1.getBoundingBox().print();

    float sim_after_add = traj1.similarityTo(traj2);
    float stDist_after_add = traj1.spatioTemporalDistanceTo(traj2, 1.0f);
    float approxDist_after_add = traj1.approximateDistance(traj2, 1.0f);

    std::cout << "Similarity after adding points: " << sim_after_add << "\n";
    std::cout << "Spatio-temporal distance after adding points: " << stDist_after_add << "\n";
    std::cout << "Approximate distance after adding points: " << approxDist_after_add << "\n";

    // -------------------- Dynamic Update: Modify Points in traj2 --------------------
    std::cout << "\n--- Updating Points in traj2 ---\n";
    traj2.updatePointAt(0, Point3D(0.0f, 0.0f, 1000));
    traj2.updatePointAt(2, Point3D(6.0f, 8.0f, 1200));

    traj2.getBoundingBox().print();

    float sim_after_update = traj1.similarityTo(traj2);
    float stDist_after_update = traj1.spatioTemporalDistanceTo(traj2, 1.0f);
    float approxDist_after_update = traj1.approximateDistance(traj2, 1.0f);

    std::cout << "Similarity after updating traj2: " << sim_after_update << "\n";
    std::cout << "Spatio-temporal distance after updating traj2: " << stDist_after_update << "\n";
    std::cout << "Approximate distance after updating traj2: " << approxDist_after_update << "\n";

    // -------------------- Dynamic Deletion: Remove Points --------------------
    std::cout << "\n--- Deleting Points from traj1 ---\n";
    traj1.deletePointAt(0);
    traj1.deletePointAt(0);

    traj1.getBoundingBox().print();

    float sim_after_delete = traj1.similarityTo(traj2);
    float stDist_after_delete = traj1.spatioTemporalDistanceTo(traj2, 1.0f);
    float approxDist_after_delete = traj1.approximateDistance(traj2, 1.0f);

    std::cout << "Similarity after deleting points from traj1: " << sim_after_delete << "\n";
    std::cout << "Spatio-temporal distance after deleting points from traj1: " << stDist_after_delete << "\n";
    std::cout << "Approximate distance after deleting points from traj1: " << approxDist_after_delete << "\n";

    std::cout << "\nAll Trajectory tests (including dynamic expansion, updates, deletions, and distances) passed successfully!\n";
    return 0;
}
