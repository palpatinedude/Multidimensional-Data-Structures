// parquet_reader.cpp - Implementation using Apache Arrow
#include "parquet_reader.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <filesystem>

bool ParquetReader::loadFromParquet(const std::string& filename) {
    try {
        std::cout << "Loading parquet file: " << filename << std::endl;
        
        // Open parquet file
        std::shared_ptr<arrow::io::ReadableFile> infile;
        auto result = arrow::io::ReadableFile::Open(filename);
        if (!result.ok()) {
            std::cout << "Error opening file: " << result.status().ToString() << std::endl;
            return false;
        }
        infile = result.ValueOrDie();

        // Create parquet reader
        std::unique_ptr<parquet::arrow::FileReader> reader;
        auto status = parquet::arrow::OpenFile(infile, arrow::default_memory_pool(), &reader);
        if (!status.ok()) {
            std::cout << "Error creating parquet reader: " << status.ToString() << std::endl;
            return false;
        }

        // Read into Arrow table
        std::shared_ptr<arrow::Table> table;
        status = reader->ReadTable(&table);
        if (!status.ok()) {
            std::cout << "Error reading table: " << status.ToString() << std::endl;
            return false;
        }

        std::cout << "Successfully loaded table with " << table->num_rows() << " rows" << std::endl;
        
        // Process the table to extract trip data
        processArrowTable(table);
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "Exception while loading parquet: " << e.what() << std::endl;
        return false;
    }
}

bool ParquetReader::loadFromParquetDirectory(const std::string& directory) {
    try {
        std::cout << "Loading parquet files from directory: " << directory << std::endl;
        
        std::vector<std::shared_ptr<arrow::Table>> tables;
        int fileCount = 0;
        
        // Iterate through all .parquet files in directory
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == ".parquet") {
                std::string filename = entry.path().string();
                std::cout << "Processing: " << filename << std::endl;
                
                // Load each parquet file
                std::shared_ptr<arrow::io::ReadableFile> infile;
                auto result = arrow::io::ReadableFile::Open(filename);
                if (!result.ok()) continue;
                infile = result.ValueOrDie();

                std::unique_ptr<parquet::arrow::FileReader> reader;
                auto status = parquet::arrow::OpenFile(infile, arrow::default_memory_pool(), &reader);
                if (!status.ok()) continue;

                std::shared_ptr<arrow::Table> table;
                status = reader->ReadTable(&table);
                if (!status.ok()) continue;
                
                tables.push_back(table);
                fileCount++;
            }
        }
        
        if (tables.empty()) {
            std::cout << "No parquet files found in directory" << std::endl;
            return false;
        }
        
        // Concatenate all tables
        auto result = arrow::ConcatenateTables(tables);
        if (!result.ok()) {
            std::cout << "Error concatenating tables: " << result.status().ToString() << std::endl;
            return false;
        }
        
        std::shared_ptr<arrow::Table> combinedTable = result.ValueOrDie();
        std::cout << "Combined " << fileCount << " files into table with " 
                  << combinedTable->num_rows() << " rows" << std::endl;
        
        processArrowTable(combinedTable);
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "Exception while loading parquet directory: " << e.what() << std::endl;
        return false;
    }
}

void ParquetReader::processArrowTable(std::shared_ptr<arrow::Table> table) {
    try {
        // Get column indices (adjust based on your parquet schema)
        int vehicleIdCol = -1, tripIdCol = -1, xCol = -1, yCol = -1, timestampCol = -1;
        
        for (int i = 0; i < table->num_columns(); i++) {
            std::string colName = table->schema()->field(i)->name();
            if (colName == "vehicle_id") vehicleIdCol = i;
            else if (colName == "trip_id") tripIdCol = i;
            else if (colName == "x") xCol = i;
            else if (colName == "y") yCol = i;
            else if (colName == "t") timestampCol = i;
        }
        
        if (tripIdCol == -1 || timestampCol == -1) {
            std::cout << "Required columns not found (trip_id, t)" << std::endl;
            return;
        }
        
        // Extract data from Arrow arrays
        auto tripIdArray = table->column(tripIdCol)->chunk(0);
        auto timestampArray = table->column(timestampCol)->chunk(0);
        
        // Group trajectory points by trip_id
        std::map<long, std::vector<long long>> tripTimestamps;
        
        for (int64_t i = 0; i < table->num_rows(); i++) {
            // Extract trip_id (assuming int32)
            auto tripIdValue = std::static_pointer_cast<arrow::Int32Array>(tripIdArray)->Value(i);
            
            // Extract timestamp (assuming int64)
            auto timestampValue = std::static_pointer_cast<arrow::Int64Array>(timestampArray)->Value(i);
            
            tripTimestamps[tripIdValue].push_back(timestampValue);
        }
        
        std::cout << "Found " << tripTimestamps.size() << " unique trips" << std::endl;
        
        // Create trip intervals
        for (const auto& [tripId, timestamps] : tripTimestamps) {
            if (timestamps.empty()) continue;
            
            auto minmax = std::minmax_element(timestamps.begin(), timestamps.end());
            long long startTime = *minmax.first;
            long long endTime = *minmax.second;
            
            trips.push_back({tripId, startTime, endTime});
            
            // Add to unique timestamps
            uniqueTimestamps.push_back(startTime);
            uniqueTimestamps.push_back(endTime);
        }
        
        // Sort and deduplicate timestamps
        std::sort(uniqueTimestamps.begin(), uniqueTimestamps.end());
        uniqueTimestamps.erase(std::unique(uniqueTimestamps.begin(), uniqueTimestamps.end()), 
                              uniqueTimestamps.end());
        
        std::cout << "Extracted " << trips.size() << " trip intervals with " 
                  << uniqueTimestamps.size() << " unique timestamps" << std::endl;
                  
    } catch (const std::exception& e) {
        std::cout << "Exception processing Arrow table: " << e.what() << std::endl;
    }
}

std::vector<std::tuple<long, long long, long long>> ParquetReader::getTrips() const {
    return trips;
}

std::vector<long long> ParquetReader::getTimestamps() const {
    return uniqueTimestamps;
}

void ParquetReader::printStats() const {
    if (trips.empty()) {
        std::cout << "No trip data loaded" << std::endl;
        return;
    }
    
    long long minStart = trips[0].get<1>();
    long long maxEnd = trips[0].get<2>();
    long long totalDuration = 0;
    
    for (const auto& trip : trips) {
        minStart = std::min(minStart, std::get<1>(trip));
        maxEnd = std::max(maxEnd, std::get<2>(trip));
        totalDuration += (std::get<2>(trip) - std::get<1>(trip));
    }
    
    std::cout << "\n=== Trip Statistics ===" << std::endl;
    std::cout << "Total trips: " << trips.size() << std::endl;
    std::cout << "Time range: " << minStart << " to " << maxEnd << std::endl;
    std::cout << "Total time span: " << (maxEnd - minStart) << " seconds" << std::endl;
    std::cout << "Average trip duration: " << (totalDuration / trips.size()) << " seconds" << std::endl;
    std::cout << "Unique timestamps: " << uniqueTimestamps.size() << std::endl;
}