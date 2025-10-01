// parquet_reader.h - Header for reading parquet files with Apache Arrow
#ifndef PARQUET_READER_H
#define PARQUET_READER_H

#include <vector>
#include <tuple>
#include <string>
#include <memory>

// Forward declarations to avoid heavy includes in header
namespace arrow { class Table; }

/**
 * ParquetReader class for loading GPS trajectory data from parquet files
 * Extracts trip intervals [start_time, end_time] for segment tree analysis
 */
class ParquetReader {
private:
    std::vector<std::tuple<long, long long, long long>> trips;  // (trip_id, start_time, end_time)
    std::vector<long long> uniqueTimestamps;

public:
    /**
     * Load trajectory data from parquet file and extract trip intervals
     * @param filename: Path to parquet file
     * @return: True if successful, false otherwise
     */
    bool loadFromParquet(const std::string& filename);
    
    /**
     * Load from directory containing multiple parquet chunk files
     * @param directory: Path to directory with .parquet files
     * @return: True if successful, false otherwise
     */
    bool loadFromParquetDirectory(const std::string& directory);
    
    /**
     * Get trip data in format expected by SegmentTree
     * @return: Vector of (trip_id, start_time, end_time) tuples
     */
    std::vector<std::tuple<long, long long, long long>> getTrips() const;
    
    /**
     * Get unique timestamps for segment tree construction
     * @return: Sorted vector of unique timestamps
     */
    std::vector<long long> getTimestamps() const;
    
    /**
     * Print statistics about loaded data
     */
    void printStats() const;

private:
    /**
     * Process Arrow table to extract trip intervals
     * @param table: Arrow table containing trajectory data
     */
    void processArrowTable(std::shared_ptr<arrow::Table> table);
    
    /**
     * Extract trip intervals from trajectory points
     * Groups by trip_id and finds start/end times
     */
    void extractTripIntervals();
};

#endif