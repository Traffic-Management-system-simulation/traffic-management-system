
#include "ConsoleInterface.hpp"

#include <iostream>
#include <sstream>
#include <string>

static int failures = 0;

#define CHECK(condition) do { \
    if (!(condition)) { \
        std::cerr << "FAIL: " #condition \
                  << " (line " << __LINE__ << ")\n"; \
        ++failures; \
    } \
} while (false)

void test_menu_and_single_tick() {
    SimulationEngine engine;
    engine.reset();

    // Start, step one tick, quit.
    std::istringstream input("1\n12\n1\n0\n");
    std::ostringstream output;

    ConsoleInterface console(engine, input, output);
    console.run();

    CHECK(engine.getCurrentTick() == 1);
    CHECK(output.str().find("Simulation started") != std::string::npos);
    CHECK(output.str().find("Goodbye") != std::string::npos);
    CHECK(output.str().find("t=") != std::string::npos);
}

void test_invalid_menu_input() {
    SimulationEngine engine;
    engine.reset();

    std::istringstream input("abc\n0\n");
    std::ostringstream output;

    ConsoleInterface console(engine, input, output);
    console.run();

    CHECK(output.str().find("Invalid input") != std::string::npos);
    CHECK(output.str().find("Goodbye") != std::string::npos);
}

void test_start_does_not_advance_ticks() {
    SimulationEngine engine;
    engine.reset();

    std::istringstream input("1\n0\n");
    std::ostringstream output;

    ConsoleInterface console(engine, input, output);
    console.run();

    CHECK(engine.getState() == SimState::RUNNING);
    CHECK(engine.getCurrentTick() == 0);
}

void test_eof_exits_cleanly() {
    SimulationEngine engine;
    engine.reset();

    std::istringstream input("");
    std::ostringstream output;

    ConsoleInterface console(engine, input, output);
    console.run();

    CHECK(output.str().find("Input closed") != std::string::npos);
}

int main() {
    test_menu_and_single_tick();
    test_invalid_menu_input();
    test_start_does_not_advance_ticks();
    test_eof_exits_cleanly();

    if (failures == 0) {
        std::cout << "All ConsoleInterface tests passed.\n";
    } else {
        std::cout << failures << " test(s) failed.\n";
    }

    return failures == 0 ? 0 : 1;
}
