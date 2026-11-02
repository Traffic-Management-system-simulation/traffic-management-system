
#include "FileManager.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << "FAIL: " #cond " (line " << __LINE__ << ")\n";    \
            ++failures;                                                    \
        }                                                                  \
    } while (0)

void test_save_and_load_settings() {
    SignalManager original;
    original.setDurations(15, 5);

    CHECK(FileManager::saveSettings(original, "test_settings.txt"));

    SignalManager loaded;
    CHECK(FileManager::loadSettings(loaded, "test_settings.txt"));
    CHECK(loaded.getGreenDuration() == 15);
    CHECK(loaded.getYellowDuration() == 5);
}


void test_invalid_settings() {
    SignalManager signals;
    signals.setDurations(15, 5);

    // Invalid duration values
    {
        std::ofstream file("test_settings.txt");
        file << "100,20\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
    CHECK(signals.getGreenDuration() == 15);
    CHECK(signals.getYellowDuration() == 5);

    // Non-numeric values
    {
        std::ofstream file("test_settings.txt");
        file << "abc,xyz\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
    CHECK(signals.getGreenDuration() == 15);
    CHECK(signals.getYellowDuration() == 5);

    // Empty file
    {
        std::ofstream file("test_settings.txt");
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
    CHECK(signals.getGreenDuration() == 15);
    CHECK(signals.getYellowDuration() == 5);

    // Extra content on a second line
    {
        std::ofstream file("test_settings.txt");
        file << "15,5\nextra\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
    CHECK(signals.getGreenDuration() == 15);
    CHECK(signals.getYellowDuration() == 5);

    // Invalid boundary values

    const char* invalidBoundaryInputs[] = {
        "0,5\n",    // Green below minimum
        "61,5\n",   // Green above maximum
        "15,0\n",   // Yellow below minimum
        "15,11\n"   // Yellow above maximum
    };

    for (const char* input : invalidBoundaryInputs) {
        {
            std::ofstream file("test_settings.txt");
            file << input;
        }

        CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));

        // Failed loads must preserve existing configuration
        CHECK(signals.getGreenDuration() == 15);
        CHECK(signals.getYellowDuration() == 5);
    }


    // Trailing content on the same line
    {
        std::ofstream file("test_settings.txt");
        file << "15,5 extra\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
    CHECK(signals.getGreenDuration() == 15);
    CHECK(signals.getYellowDuration() == 5);

    // Missing comma
    {
        std::ofstream file("test_settings.txt");
        file << "15 5\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
    CHECK(signals.getGreenDuration() == 15);
    CHECK(signals.getYellowDuration() == 5);
}


void test_missing_settings_file() {
    SignalManager signals;
    CHECK(!FileManager::loadSettings(signals, "missing_settings.txt"));
}

void test_csv_export() {
    StatisticsManager stats;

    Vehicle v1{};
    v1.entryTick = 0;

    Vehicle v2{};
    v2.entryTick = 2;

    stats.recordProcessedVehicle(v1, 5);
    stats.recordProcessedVehicle(v2, 8);

    int processedBy[4] = {1, 0, 1, 0};
    size_t waitingByApproach[4] = {2, 1, 0, 0};

    CHECK(FileManager::exportStatistics(
       stats, 10, 3, processedBy, waitingByApproach,
       "test_statistics.csv"));

    std::ifstream file("test_statistics.csv");
    CHECK(file.is_open());

    std::string contents(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    CHECK(contents.find("North Vehicles Processed,1") != std::string::npos);
    CHECK(contents.find("South Vehicles Processed,0") != std::string::npos);
    CHECK(contents.find("East Vehicles Processed,1") != std::string::npos);
    CHECK(contents.find("West Vehicles Processed,0") != std::string::npos);

    CHECK(contents.find("North Vehicles Waiting,2") != std::string::npos);
    CHECK(contents.find("South Vehicles Waiting,1") != std::string::npos);
    CHECK(contents.find("East Vehicles Waiting,0") != std::string::npos);
    CHECK(contents.find("West Vehicles Waiting,0") != std::string::npos);

    CHECK(contents.find("Total Simulation Ticks,10") != std::string::npos);
    CHECK(contents.find("Vehicles Processed,2") != std::string::npos);
    CHECK(contents.find("Vehicles Still Waiting,3") != std::string::npos);
    CHECK(contents.find("Average Waiting Time,5.50") != std::string::npos);
    CHECK(contents.find("Throughput,0.20") != std::string::npos);

    file.close();
    std::remove("test_statistics.csv");
}

int main() {
    test_save_and_load_settings();
    test_invalid_settings();
    test_missing_settings_file();
    test_csv_export();

    std::remove("test_settings.txt");

    if (failures == 0)
        std::cout << "All FileManager tests passed.\n";
    else
        std::cout << failures << " check(s) failed.\n";

    return failures == 0 ? 0 : 1;
}
