# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

# Source files
SRC = main.cpp \
      common.cpp \
      methods/2D/divideConquer.cpp \
      methods/3D/divideConquer.cpp \
      methods/2D/monotoneChain.cpp \
      methods/2D/quickHull.cpp \
      methods/3D/quickHull.cpp \
      methods/2D/grahamScan.cpp \
      methods/3D/incremental.cpp \
      evaluation/evaluation.cpp 

# Object files
OBJ = $(SRC:.cpp=.o)

# Executable
TARGET = convexHullExperiments

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean object files and executable
clean:
	rm -f $(OBJ) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)
