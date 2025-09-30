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
        
        auto result = arrow::io::ReadableFile::Open(filename);
        if (!result.ok()) {
            std::cout << "Error opening file: " << result.status().message() << std::endl;
            return false;
        }
        std::shared_ptr<arrow::io::ReadableFile> infile = result.ValueOrDie();

        auto reader_result = parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
        if (!reader_result.ok()) {
            std::cout << "Error creating parquet reader: " << reader_result.status().message() << std::endl;
            return false;
        }
        std::unique_ptr<parquet::arrow::FileReader> reader = std::move(reader_result).ValueOrDie();

        std::shared_ptr<arrow::Table> table;
        auto status = reader->ReadTable(&table);
        if (!status.ok()) {
            std::cout << "Error reading table: " << status.message() << std::endl;
            return false;
        }

        std::cout << "Successfully loaded table with " << table->num_rows() << " rows" << std::endl;
        
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
        
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == ".parquet") {
                std::string filename = entry.path().string();
                std::cout << "Processing: " << filename << std::endl;
                
                auto result = arrow::io::ReadableFile::Open(filename);
                if (!result.ok()) {
                    std::cout << "  Skipping file, error: " << result.status().message() << std::endl;
                    continue;
                }
                std::shared_ptr<arrow::io::ReadableFile> infile = result.ValueOrDie();

                auto reader_result = parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
                if (!reader_result.ok()) {
                    std::cout << "  Skipping file, reader error: " << reader_result.status().message() << std::endl;
                    continue;
                }
                std::unique_ptr<parquet::arrow::FileReader> reader = std::move(reader_result).ValueOrDie();

                std::shared_ptr<arrow::Table> table;
                auto status = reader->ReadTable(&table);
                if (!status.ok()) {
                    std::cout << "  Skipping file, read error: " << status.message() << std::endl;
                    continue;
                }
                
                std::cout << "  Loaded " << table->num_rows() << " rows" << std::endl;
                tables.push_back(table);
                fileCount++;
            }
        }
        
        if (tables.empty()) {
            std::cout << "No parquet files found in directory" << std::endl;
            return false;
        }
        
        auto result = arrow::ConcatenateTables(tables);
        if (!result.ok()) {
            std::cout << "Error concatenating tables: " << result.status().message() << std::endl;
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
        int vehicleIdCol = -1, tripIdCol = -1, timestampCol = -1;
        
        for (int i = 0; i < table->num_columns(); i++) {
            std::string colName = table->schema()->field(i)->name();
            if (colName == "vehicle_id") vehicleIdCol = i;
            else if (colName == "trip_id") tripIdCol = i;
            else if (colName == "t") timestampCol = i;
        }
        
        if (tripIdCol == -1 || timestampCol == -1) {
            std::cout << "Required columns not found (trip_id, t)" << std::endl;
            return;
        }
        
        auto tripIdArray = table->column(tripIdCol)->chunk(0);
        auto timestampArray = table->column(timestampCol)->chunk(0);
        
        std::map<long, std::vector<long long>> tripTimestamps;
        
        // Valid timestamp range for 2018-2019 dataset (Unix timestamps)
        const long long MIN_VALID_TIMESTAMP = 1514764800;  // 2018-01-01
        const long long MAX_VALID_TIMESTAMP = 1577836800;  // 2020-01-01
        
        int invalidCount = 0;
        
        for (int64_t i = 0; i < table->num_rows(); i++) {
            auto tripIdValue = std::static_pointer_cast<arrow::Int32Array>(tripIdArray)->Value(i);
            auto timestampValue = std::static_pointer_cast<arrow::Int64Array>(timestampArray)->Value(i);
            
            // Filter out invalid timestamps
            if (timestampValue < MIN_VALID_TIMESTAMP || timestampValue > MAX_VALID_TIMESTAMP) {
                invalidCount++;
                continue;
            }
            
            tripTimestamps[tripIdValue].push_back(timestampValue);
        }
        
        std::cout << "Found " << tripTimestamps.size() << " unique trips" << std::endl;
        std::cout << "Filtered out " << invalidCount << " invalid timestamp records" << std::endl;
        
        int validTrips = 0;
        int invalidTrips = 0;
        
        for (const auto& [tripId, timestamps] : tripTimestamps) {
            if (timestamps.empty()) continue;
            
            auto minmax = std::minmax_element(timestamps.begin(), timestamps.end());
            long long startTime = *minmax.first;
            long long endTime = *minmax.second;
            
            // Additional validation: trip must have positive duration
            if (endTime <= startTime) {
                invalidTrips++;
                continue;
            }
            
            // Additional validation: trip duration should be reasonable (max 24 hours)
            if (endTime - startTime > 86400) {
                invalidTrips++;
                continue;
            }
            
            trips.push_back({tripId, startTime, endTime});
            uniqueTimestamps.push_back(startTime);
            uniqueTimestamps.push_back(endTime);
            validTrips++;
        }
        
        std::sort(uniqueTimestamps.begin(), uniqueTimestamps.end());
        uniqueTimestamps.erase(std::unique(uniqueTimestamps.begin(), uniqueTimestamps.end()), 
                              uniqueTimestamps.end());
        
        std::cout << "Extracted " << validTrips << " valid trip intervals (filtered " 
                  << invalidTrips << " invalid trips)" << std::endl;
        std::cout << "Unique timestamps: " << uniqueTimestamps.size() << std::endl;
                  
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
    
    long long minStart = std::get<1>(trips[0]);
    long long maxEnd = std::get<2>(trips[0]);
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