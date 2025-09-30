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
    std::cout << "Segment Tree with Parquet Data" << std::endl;
    std::cout << "==============================" << std::endl;
    
    ParquetReader reader;
    
    // Load from parquet directory with absolute path
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
    
    // Limit data size for testing
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
    
    // Test queries
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
    
    // Performance analysis with random queries
    std::cout << "\n=== Performance Analysis ===" << std::endl;
    
    if (!timestamps.empty()) {
        long long minTime = timestamps.front();
        long long maxTime = timestamps.back();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long long> timeDist(minTime, maxTime);
        
        int numQueries = 1000;
        auto perfStart = std::chrono::high_resolution_clock::now();
        
        int totalTrips = 0;
        for (int i = 0; i < numQueries; i++) {
            long long queryStart = timeDist(gen);
            long long queryEnd = queryStart + (maxTime - minTime) / 20;
            totalTrips += st.query(queryStart, std::min(queryEnd, maxTime));
        }
        
        auto perfEnd = std::chrono::high_resolution_clock::now();
        auto perfTime = std::chrono::duration_cast<std::chrono::microseconds>(perfEnd - perfStart).count();
        
        std::cout << "Executed " << numQueries << " random queries" << std::endl;
        std::cout << "Total time: " << perfTime << " microseconds" << std::endl;
        std::cout << "Average time per query: " << (perfTime / numQueries) << " microseconds" << std::endl;
        std::cout << "Average trips found: " << (totalTrips / numQueries) << std::endl;
        
        // Theoretical complexity
        double n = timestamps.size();
        double logN = std::log2(n);
        std::cout << "\nComplexity Analysis:" << std::endl;
        std::cout << "n (timestamps): " << (int)n << std::endl;
        std::cout << "Theoretical build: O(n log n) ≈ " << (int)(n * logN) << std::endl;
        std::cout << "Theoretical query: O(log n) ≈ " << (int)logN << std::endl;
    }
    
    std::cout << "\nSegment tree analysis complete!" << std::endl;
    return 0; }