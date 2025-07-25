# Makefile for compiling test_point3d_from_csv.cpp

CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./include

SRC = src/point3D.cpp src/bbox3D.cpp src/RTreeNode.cpp src/trajectory.cpp src/RTree.cpp
TEST = test/test_points_bbox.cpp test/test_all.cpp
TARGET = test_points_bbox test_all
DATA = data/trajectory_data.csv

all: $(TARGET)

$(TARGET): $(SRC) $(TEST)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
