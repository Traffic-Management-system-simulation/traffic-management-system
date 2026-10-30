
#include "FileManager.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>

// Save signal duration settings to a file.
bool FileManager::saveSettings(const SignalManager& signals,
                               const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    file << signals.getGreenDuration() << ","
         << signals.getYellowDuration() << "\n";

    return file.good();
}

// Load and validate signal duration settings.
bool FileManager::loadSettings(SignalManager& signals,
                               const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) {
        return false;
    }

    std::stringstream input(line);
    int green = 0;
    int yellow = 0;
    char comma = '\0';
    std::string extra;

    if (!(input >> green >> comma >> yellow) ||
        comma != ',' || (input >> extra)) {
        return false;
    }

    // Validate values before applying them.
    if (green < 1 || green > 60 ||
        yellow < 1 || yellow > 10) {
        return false;
    }

    signals.setDurations(green, yellow);
    return true;
}

// Export aggregate simulation statistics to a CSV file.
bool FileManager::exportStatistics(const StatisticsManager& stats,
                                   int totalTicks,
                                   int waitingVehicles,
                                   const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    file << "Metric,Value\n";
    file << "Total Simulation Ticks," << totalTicks << "\n";
    file << "Vehicles Processed," << stats.getProcessedCount() << "\n";
    file << "Vehicles Still Waiting," << waitingVehicles << "\n";

    file << std::fixed << std::setprecision(2);

    file << "Average Waiting Time,"
         << stats.calculateAverageWaitTime() << "\n";

    file << "Throughput,"
         << stats.calculateThroughput(totalTicks) << "\n";

    return file.good();
}
