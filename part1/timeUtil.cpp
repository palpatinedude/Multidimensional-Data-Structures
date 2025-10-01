
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdint>

namespace timeUtil {
    int64_t parseTimestampToSeconds(const std::string& ts) {
        std::string tsClean = ts;
        if (!ts.empty() && ts.back() == 'Z') tsClean.pop_back(); // remove Z if present

        std::tm tm{};
        std::istringstream ss(tsClean);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");  // ISO 8601 without Z
        if (ss.fail()) {
            std::cerr << "[Error] Failed to parse timestamp: " << ts << "\n";
            return 0;
        }

    #if defined(_WIN32) || defined(_WIN64)
        time_t timeEpoch = _mkgmtime(&tm);  // Windows UTC
    #else
        time_t timeEpoch = timegm(&tm);     // POSIX UTC
    #endif

        if (timeEpoch == -1) {
            std::cerr << "[Error] Failed to convert timestamp to epoch: " << ts << "\n";
            return 0;
        }

        return static_cast<int64_t>(timeEpoch);
    }
}