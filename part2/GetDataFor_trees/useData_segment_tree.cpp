// main_with_parquet.cpp - Main function using parquet data to fill the segment tree
#include <iostream>
#include <chrono>
#include "segment_tree.h"
#include "parquet_reader.h"

int main() {
    std::cout << "Segment Tree with Parquet Data" << std::endl;
    std::cout << "==============================" << std::endl;
    
    ParquetReader reader;
    
    // Try to load from single parquet file first
    bool loaded = reader.loadFromParquet("trajectories_grouped.parquet");
    
    // If that fails, try the parquet chunks directory
    if (!loaded) {
        std::cout << "Single file not found, trying directory..." << std::endl;
        loaded = reader.loadFromParquetDirectory("trajectory_data_parquet");
    }
    
    if (!loaded) {
        std::cout << "Failed to load any parquet data!" << std::endl;
        return 1;
    }
    
    // Print data statistics
    reader.printStats();
    
    // Get data for segment tree
    auto trips = reader.getTrips();
    auto timestamps = reader.getTimestamps();
    
    if (trips.empty()) {
        std::cout << "No trip data to process!" << std::endl;
        return 1;
    }
    
    // Limit data size for testing (optional)
    if (trips.size() > 5000) {
        trips.resize(5000);
        std::cout << "Limited to first 5000 trips for testing" << std::endl;
    }
    
    // Build segment tree
    std::cout << "\n=== Building Segment Tree ===" << std::endl;
    auto buildStart = std::chrono::high_resolution_clock::now();
    
    SegmentTree st(timestamps, trips);
    
    auto buildEnd = std::chrono::high_resolution_clock::now();
    auto buildTime = std::chrono::duration_cast<std::chrono::milliseconds>(buildEnd - buildStart).count();
    
    std::cout << "Segment tree built in " << buildTime << " milliseconds" << std::endl;
    
    // Test some queries
    std::cout << "\n=== Sample Queries ===" << std::endl;
    
    if (!timestamps.empty()) {
        long long minTime = timestamps.front();
        long long maxTime = timestamps.back();
        long long quarterTime = (maxTime - minTime) / 4;
        
        struct TestQuery {
            long long start, end;
            std::string description;
        };
        
        std::vector<TestQuery> queries = {
            {minTime, minTime + quarterTime, "First quarter"},
            {minTime + quarterTime, minTime + 2 * quarterTime, "Second quarter"},
            {minTime + 2 * quarterTime, minTime + 3 * quarterTime, "Third quarter"},
            {minTime + 3 * quarterTime, maxTime, "Last quarter"},
            {minTime, maxTime, "Full range"}
        };
        
        for (const auto& query : queries) {
            auto queryStart = std::chrono::high_resolution_clock::now();
            int result = st.query(query.start, query.end);
            auto queryEnd = std::chrono::high_resolution_clock::now();
            auto queryTime = std::chrono::duration_cast<std::chrono::microseconds>(queryEnd - queryStart).count();
            
            std::cout << query.description << " [" << query.start << ", " << query.end << "]: " 
                      << result << " trips (" << queryTime << " μs)" << std::endl;
        }
    }
    
    return 0;
}