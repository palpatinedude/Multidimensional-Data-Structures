CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./api/include

# Arrow/Parquet include and library paths (adjust these to your installation)
ARROW_INC = /usr/local/include
ARROW_LIB = /usr/local/lib
PARQUET_LIBS = -larrow -lparquet -lz -lsnappy -llz4 -lbz2 -pthread

# Source files
SRC = main.cpp timeUtil.cpp \
      api/src/point3D.cpp \
      api/src/bbox3D.cpp \
      api/src/trajectory.cpp \
      api/src/RTreeNode.cpp \
      api/src/RTree.cpp \
      evaluation/evaluation.cpp 

# Object files
OBJ = $(SRC:.cpp=.o)

# Executable name
TARGET = main

# Default rule
all: $(TARGET)

# Link objects to create executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(ARROW_INC) -L$(ARROW_LIB) -o $@ $(OBJ) $(PARQUET_LIBS)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(ARROW_INC) -c $< -o $@

# Compile and run the program
run: $(TARGET)
	./$(TARGET)

# Clean compiled files
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean run
