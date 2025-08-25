
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdint>

namespace timeUtil {
    int32_t parseTimestampToSeconds(const std::string& ts) {
        std::tm tm{};
        std::istringstream ss(ts);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");  // ISO 8601 format with Z for UTC
        if (ss.fail()) {
            std::cerr << "[Error] Failed to parse timestamp: " << ts << "\n";
            return 0;
        }

    #if defined(_WIN32) || defined(_WIN64)
        time_t timeEpoch = _mkgmtime(&tm);  // Windows version of timegm
    #else
        time_t timeEpoch = timegm(&tm);     // POSIX function to convert UTC tm to epoch
    #endif

        if (timeEpoch == -1) {
            std::cerr << "[Error] Failed to convert timestamp to epoch: " << ts << "\n";
            return 0;
        }

        return static_cast<int32_t>(timeEpoch);
    }
}