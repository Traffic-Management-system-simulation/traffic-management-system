#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <string>
#include "SignalManager.hpp"
#include "StatisticsManager.hpp"

class FileManager {
public:
    static bool saveSettings(const SignalManager& signals,
                             const std::string& filename);

    static bool loadSettings(SignalManager& signals,
                             const std::string& filename);

    static bool exportStatistics(const StatisticsManager& stats,
                                 int totalTicks,
                                 int waitingVehicles,
                                 const std::string& filename);
};

#endif // FILE_MANAGER_HPP