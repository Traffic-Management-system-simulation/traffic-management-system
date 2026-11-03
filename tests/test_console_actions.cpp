// Drives ConsoleInterface with scripted menu input (TC-SIM, TC-UX, TC-SEC input handling)
#include "ConsoleInterface.hpp"

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

// Runs one scripted session and returns everything the console printed.
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

void test_state_controls() {
    { SimulationEngine e; e.reset(); run(e, "1\n2\n0\n");
      CHECK(e.getState() == SimState::PAUSED); }
    { SimulationEngine e; e.reset(); run(e, "1\n2\n3\n0\n");
      CHECK(e.getState() == SimState::RUNNING); }
    { SimulationEngine e; e.reset(); run(e, "1\n5\n0\n");
      CHECK(e.getState() == SimState::STOPPED); }
    { SimulationEngine e; e.reset(); run(e, "1\n12\n3\n4\n0\n");
      CHECK(e.getCurrentTick() == 0);
      CHECK(e.getState() == SimState::STOPPED); }
}

void test_actions_in_wrong_state() {
    // Pause, Resume and Stop while STOPPED must be rejected, not crash.
    SimulationEngine e; e.reset();
    std::string out = run(e, "2\n3\n5\n0\n");
    CHECK(has(out, "Goodbye"));
    CHECK(e.getState() == SimState::STOPPED);

    // Start twice, Start while paused.
    SimulationEngine e2; e2.reset();
    out = run(e2, "1\n1\n2\n1\n0\n");
    CHECK(has(out, "Goodbye"));
    CHECK(e2.getState() == SimState::PAUSED);

    // Step while not running does not advance the clock.
    SimulationEngine e3; e3.reset();
    run(e3, "12\n0\n");
    CHECK(e3.getCurrentTick() == 0);
}

void test_add_vehicle() {
    SimulationEngine e; e.reset();
    std::string out = run(e, "6\n1\n3\n0\n");
    CHECK(e.getQueueManager().getQueueSize(Approach::NORTH) == 3);
    CHECK(has(out, "Goodbye"));

    SimulationEngine e2; e2.reset();
    out = run(e2, "6\n9\n2\n2\n0\n");          // bad approach, then South x2
    CHECK(has(out, "Invalid input"));
    CHECK(e2.getQueueManager().getQueueSize(Approach::SOUTH) == 2);
}

void test_auto_generation() {
    SimulationEngine e; e.reset();
    std::string on = run(e, "7\n1.0\n0\n");
    CHECK(has(on, "enabled"));
    CHECK(has(on, "p=1"));

    SimulationEngine e2; e2.reset();
    std::string off = run(e2, "7\n1.0\n7\n0\n");
    CHECK(has(off, "disabled"));

    SimulationEngine e3; e3.reset();
    run(e3, "7\n1.0\n1\n12\n5\n0\n");
    int total = static_cast<int>(e3.getQueueManager().getTotalWaitingCount())
              + e3.getStatsManager().getProcessedCount();
    CHECK(e3.getCurrentTick() == 5);
    CHECK(total > 0);

    SimulationEngine e4; e4.reset();
    std::string out = run(e4, "7\n5\n0.5\n0\n");
    CHECK(has(out, "Invalid input"));
    CHECK(has(out, "enabled"));
}

void test_durations_and_signal_mode() {
    SimulationEngine e; e.reset();
    std::string out = run(e, "8\n5\n2\n0\n");
    CHECK(!has(out, "Invalid input"));
    CHECK(has(out, "Goodbye"));

    SimulationEngine e2; e2.reset();
    out = run(e2, "8\n0\n5\n2\n0\n");          // 0 is invalid, then valid values
    CHECK(has(out, "Invalid input"));

    SimulationEngine e3; e3.reset();
    run(e3, "9\n2\n3\n0\n");                   // manual mode, East green
    CHECK(e3.getSignalManager().getSignalState(Approach::EAST) == SignalColor::GREEN);
    CHECK(e3.getSignalManager().getSignalState(Approach::NORTH) != SignalColor::GREEN);

    SimulationEngine e4; e4.reset();
    out = run(e4, "9\n2\n3\n9\n1\n0\n");       // manual, then back to automatic
    CHECK(has(out, "Goodbye"));
}

void test_views_and_full_flow() {
    SimulationEngine e; e.reset();
    std::string out = run(e, "10\n11\n0\n");
    CHECK(has(out, "Goodbye"));

    // 3 North vehicles, start, 10 ticks, view, statistics, stop.
    SimulationEngine e2; e2.reset();
    out = run(e2, "6\n1\n3\n1\n12\n10\n10\n11\n5\n0\n");
    CHECK(e2.getStatsManager().getProcessedCount() == 3);
    CHECK(e2.getQueueManager().getTotalWaitingCount() == 0);
    CHECK(e2.getState() == SimState::STOPPED);
    CHECK(has(out, "Goodbye"));
}

int main() {
    test_state_controls();
    test_actions_in_wrong_state();
    test_add_vehicle();
    test_auto_generation();
    test_durations_and_signal_mode();
    test_views_and_full_flow();

    if (failures == 0) std::cout << "All console action tests passed.\n";
    else std::cout << failures << " console action test(s) failed.\n";
    return failures == 0 ? 0 : 1;
}
