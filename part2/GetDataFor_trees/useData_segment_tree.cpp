// main_with_parquet.cpp - Main function using parquet data to fill the segment tree
#include <iostream>
#include <chrono>
#include <random>     
#include <algorithm>
#include <cmath>       
#include <vector>      
#include "/home/alex/desktop/Multidimensional-Data-Structures/part2/part2.2/segment_tree.h"
#include "/home/alex/desktop/Multidimensional-Data-Structures/part2/part2.2/segment_tree.cpp"
#include "parquet_reader.h"

int main() {
    std::cout << "Segment Tree Build Test with Full Dataset" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    ParquetReader reader;
    
    // Load parquet data
    bool loaded = reader.loadFromParquetDirectory("/home/alex/desktop/Multidimensional-Data-Structures/preprocessing/trajectories_grouped.parquet");
    
    if (!loaded) {
        std::cout << "Failed to load parquet data!" << std::endl;
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
    
    std::cout << "\n=== Building Segment Tree ===" << std::endl;
    std::cout << "Number of trips (m): " << trips.size() << std::endl;
    std::cout << "Number of timestamps (n): " << timestamps.size() << std::endl;
    
    auto buildStart = std::chrono::high_resolution_clock::now();
    
    SegmentTree st(timestamps, trips);
    
    auto buildEnd = std::chrono::high_resolution_clock::now();
    auto buildTime = std::chrono::duration_cast<std::chrono::milliseconds>(buildEnd - buildStart).count();
    
    std::cout << "Segment tree built in " << buildTime << " milliseconds" << std::endl;
    
    // Test sample queries
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
    
    // Complexity analysis
    std::cout << "\n=== Complexity Analysis ===" << std::endl;
    int n = timestamps.size();
    int m = trips.size();
    
    std::cout << "Implementation build complexity: O(n × m)" << std::endl;
    std::cout << "  where n = " << n << " timestamps" << std::endl;
    std::cout << "        m = " << m << " trips" << std::endl;
    std::cout << "  Theoretical operations: " << (n * m) << std::endl;
    std::cout << "  Actual build time: " << buildTime << " ms" << std::endl;
    std::cout << "  Time per operation: " << (buildTime * 1000.0) / (n * m) << " microseconds" << std::endl;
    
    std::cout << "\nQuery complexity: O(log n + k)" << std::endl;
    std::cout << "  where n = " << n << " timestamps" << std::endl;
    std::cout << "        k = number of results" << std::endl;
    std::cout << "  Theoretical tree depth: " << (int)std::log2(n) << std::endl;
    
    std::cout << "\nNote: Standard segment tree build is O(n), but this implementation" << std::endl;
    std::cout << "      rescans all trips at each node, resulting in O(n × m) complexity." << std::endl;
    
    return 0;
}