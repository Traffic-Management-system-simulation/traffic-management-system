#include <cstring>
#include <iostream>

#include "ConsoleInterface.hpp"
#include "SimulationEngine.hpp"

int main(int argc, char* argv[]) {
    bool colorEnabled = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--color") == 0) {
            colorEnabled = true;
        }
    }

    std::cout << "--- Traffic Management System Simulation ---\n";

    SimulationEngine engine;
    engine.reset();

    ConsoleInterface console(
        engine, std::cin, std::cout, colorEnabled);

    console.run();

    return 0;
}