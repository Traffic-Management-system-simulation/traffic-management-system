
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

    {
        std::ofstream file("test_settings.txt");
        file << "100,20\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));

    {
        std::ofstream file("test_settings.txt");
        file << "abc,xyz\n";
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));

    {
        std::ofstream file("test_settings.txt");
    }
    CHECK(!FileManager::loadSettings(signals, "test_settings.txt"));
}

void test_missing_settings_file() {
    SignalManager signals;
    CHECK(!FileManager::loadSettings(signals, "missing_settings.txt"));
}

int main() {
    test_save_and_load_settings();
    test_invalid_settings();
    test_missing_settings_file();

    std::remove("test_settings.txt");

    if (failures == 0)
        std::cout << "All FileManager tests passed.\n";
    else
        std::cout << failures << " check(s) failed.\n";

    return failures == 0 ? 0 : 1;
}
