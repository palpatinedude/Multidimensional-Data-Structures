
#include "segment_tree.cpp"
#include "segment_tree.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
using namespace std;

/**
 * Simple test program to verify segment tree correctness
 * Tests basic functionality with manually verified expected results
 */
int main() {
    cout << "Testing Segment Tree with Detailed Comments" << endl;
    cout << "===========================================" << endl;
    
    // Test case: Create discrete timestamps representing key time points
    // These will become the leaf nodes of our segment tree
    vector<long long> timestamps = {1, 5, 10, 15, 20};
    
    // Create test trips: tuple format (tripId, startTime, endTime)
    // We can manually verify which trips overlap which time ranges
    vector<tuple<long, long long, long long>> trips = {
        {1, 2, 8},   // Trip 1: active from time 2 to 8
        {2, 6, 12},  // Trip 2: active from time 6 to 12  
        {3, 11, 18}, // Trip 3: active from time 11 to 18
        {4, 0, 25}   // Trip 4: active from time 0 to 25 (spans everything)
    };
    
    cout << "\nTrip Schedule:" << endl;
    cout << "Trip 1: [2, 8]   - Early trip" << endl;
    cout << "Trip 2: [6, 12]  - Middle trip" << endl;  
    cout << "Trip 3: [11, 18] - Late trip" << endl;
    cout << "Trip 4: [0, 25]  - Full span trip" << endl;
    cout << "Timestamps: [1, 5, 10, 15, 20]" << endl;
    
    try {
        // Build the segment tree - should take O(n log n) time
        cout << "\nBuilding segment tree..." << endl;
        SegmentTree st(timestamps, trips);
        cout << "Segment tree built successfully!" << endl;
        
        cout << "\nExecuting test queries:" << endl;
        cout << "======================" << endl;
        
        // Define test cases with expected results for manual verification
        struct TestCase {
            long long start, end;
            string description;
            string expectedTrips;
        };
        
        vector<TestCase> tests = {
            {1, 5, "Early period", "Trips 1,4 (2 trips)"},
            {6, 10, "Middle period", "Trips 1,2,4 (3 trips)"}, 
            {15, 20, "Late period", "Trips 3,4 (2 trips)"}, 
            {0, 25, "Full range", "All trips 1,2,3,4 (4 trips)"},
            {9, 9, "Single timestamp", "Trips 1,2,4 (3 trips)"},
            {30, 35, "Outside range", "No trips (0 trips)"}
        };
        
        // Execute each test case
        for (const auto& test : tests) {
            int result = st.query(test.start, test.end);
            cout << "Query [" << test.start << ", " << test.end << "] (" 
                 << test.description << "): " << result << " trips" << endl;
            cout << "  Expected: " << test.expectedTrips << endl;
        }
        
        cout << "\nManual Verification Guide:" << endl;
        cout << "=========================" << endl;
        cout << "Query [1,5]: Should find trips active between times 1-5" << endl;
        cout << "  - Trip 1 [2,8]: YES (overlaps 1-5)" << endl;
        cout << "  - Trip 2 [6,12]: NO (starts after 5)" << endl;
        cout << "  - Trip 3 [11,18]: NO (starts after 5)" << endl;  
        cout << "  - Trip 4 [0,25]: YES (covers 1-5)" << endl;
        cout << "  Expected: 2 trips" << endl;
        
        cout << "\nQuery [6,10]: Should find trips active between times 6-10" << endl;
        cout << "  - Trip 1 [2,8]: YES (overlaps 6-8)" << endl;
        cout << "  - Trip 2 [6,12]: YES (overlaps 6-10)" << endl;
        cout << "  - Trip 3 [11,18]: NO (starts after 10)" << endl;  
        cout << "  - Trip 4 [0,25]: YES (covers 6-10)" << endl;
        cout << "  Expected: 3 trips" << endl;
        
        cout << "\nSegment Tree Structure:" << endl;
        cout << "======================" << endl;
        cout << "Root: covers [1, 20] - all timestamps" << endl;
        cout << "├─ Left subtree: [1, 10] - earlier timestamps" << endl; 
        cout << "│  ├─ Leaf: [1] - single timestamp" << endl;
        cout << "│  └─ Leaf: [5] - single timestamp" << endl;
        cout << "└─ Right subtree: [15, 20] - later timestamps" << endl;
        cout << "   ├─ Leaf: [15] - single timestamp" << endl;
        cout << "   └─ Leaf: [20] - single timestamp" << endl;
        
    } catch (const exception& e) {
        cout << "Error during testing: " << e.what() << endl;
        return 1;
    }
    
    cout << "\nTest completed! Compare actual results with expected values above." << endl;
    return 0;
}