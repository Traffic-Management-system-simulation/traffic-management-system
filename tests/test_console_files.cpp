// Menu options 13-15: save settings, load settings, export statistics CSV (TC-DATA-01..03, TC-SEC-03)
#include "ConsoleInterface.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static int failures = 0;

#define CHECK(condition) do { \
    if (!(condition)) { \
        std::cerr << "FAIL: " #condition " (line " << __LINE__ << ")\n"; \
        ++failures; \
    } \
} while (false)

static const char* kSettings = "simulation_settings.txt";
static const char* kCsv = "simulation_statistics.csv";

static std::string run(SimulationEngine& engine, const std::string& script) {
    std::istringstream input(script);
    std::ostringstream output;
    ConsoleInterface console(engine, input, output);
    console.run();
    return output.str();
}

static bool has(const std::string& text, const char* part) {
    return text.find(part) != std::string::npos;
}

static bool fileExists(const char* path) {
    std::ifstream f(path);
    return f.good();
}

static void cleanup() {
    std::remove(kSettings);
    std::remove(kCsv);
}

void test_save_and_load_settings() {
    cleanup();
    SimulationEngine e; e.reset();

    // Set durations first (option 8), then save (13)
    std::string out = run(e, "8\n7\n2\n13\n0\n");
    CHECK(has(out, "Settings saved successfully"));
    CHECK(fileExists(kSettings));

    // Load them back (14)
    SimulationEngine e2; e2.reset();
    out = run(e2, "14\n0\n");
    CHECK(has(out, "Settings loaded successfully"));
    cleanup();
}

void test_load_missing_and_malformed() {
    cleanup();
    SimulationEngine e; e.reset();
    std::string out = run(e, "14\n0\n");                 // file does not exist
    CHECK(has(out, "Error: Could not load settings"));
    CHECK(has(out, "Goodbye"));                          // no crash

    { std::ofstream f(kSettings); f << "this is not a valid settings file\n"; }
    SimulationEngine e2; e2.reset();
    out = run(e2, "14\n0\n");                            // malformed file
    CHECK(has(out, "Error: Could not load settings"));
    CHECK(has(out, "Goodbye"));
    cleanup();
}

void test_export_statistics() {
    cleanup();
    SimulationEngine e; e.reset();
    // 3 vehicles, start, 10 ticks, export (15), quit
    std::string out = run(e, "6\n1\n3\n1\n12\n10\n15\n0\n");
    CHECK(has(out, "Statistics exported to simulation_statistics.csv"));
    CHECK(fileExists(kCsv));

    std::ifstream f(kCsv);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    CHECK(!content.empty());
    cleanup();
}

int main() {
    test_save_and_load_settings();
    test_load_missing_and_malformed();
    test_export_statistics();
    cleanup();

    if (failures == 0) std::cout << "All console file tests passed.\n";
    else std::cout << failures << " console file test(s) failed.\n";
    return failures == 0 ? 0 : 1;
}
