# TODO List for Computing Skyline Operator Subsets

## Overview
Calculating the Skyline Operator subsets for a set of 2D points (scientists) with dimensions: number of awards and DBLP records.

## Tasks

### 1. Data Preparation
- [ ] Collect data for each scientist (#Awards, #DBLP_Record).
- [ ] Format data into 2D points.

### 2. Compute Skyline Subsets
- [ ] Implement an algorithm to identify the Skyline of 2D points.
- [ ] Compute 4 subsets:
  - Subset 1: MIN #Awards, MIN #DBLP_Record
  - Subset 2: MIN #Awards, MAX #DBLP_Record
  - Subset 3: MAX #Awards, MIN #DBLP_Record
  - Subset 4: MAX #Awards, MAX #DBLP_Record

### 3. Visualization
- [ ] Develop a method to visualize the Skyline and its subsets.
- [ ] Ensure clarity in showing different subsets.

### 4. Testing and Validation
- [ ] Test the algorithm with sample datasets.
- [ ] Validate that the subsets conform to the Skyline Operator's definition.

### 5. Documentation
- [ ] Document the methodology and algorithm.
- [ ] Provide examples and use cases.
- [ ] Explain the significance and applications of the Skyline Operator.

## Notes
- Focus on the geometric and database aspects of the problem.
- Visualization is key for understanding and presenting results.
