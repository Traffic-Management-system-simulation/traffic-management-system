
#include "ConsoleInterface.hpp"
#include "InputValidator.hpp"

#include <iomanip>
#include <iostream>
#include <string>

namespace {

const Approach APPROACHES[] = {
    Approach::NORTH,
    Approach::SOUTH,
    Approach::EAST,
    Approach::WEST
};

const char* approachName(Approach a) {
    switch (a) {
        case Approach::NORTH: return "NORTH";
        case Approach::SOUTH: return "SOUTH";
        case Approach::EAST:  return "EAST";
        case Approach::WEST:  return "WEST";
    }
    return "UNKNOWN";
}

const char* signalName(SignalColor s) {
    switch (s) {
        case SignalColor::RED:    return "RED";
        case SignalColor::YELLOW: return "YELLOW";
        case SignalColor::GREEN:  return "GREEN";
    }
    return "UNKNOWN";
}

const char* stateName(SimState s) {
    switch (s) {
        case SimState::STOPPED: return "STOPPED";
        case SimState::RUNNING: return "RUNNING";
        case SimState::PAUSED:  return "PAUSED";
    }
    return "UNKNOWN";
}

std::string paint(const std::string& value, const char* code, bool enabled) {
    if (!enabled) return value;
    return std::string(code) + value + "\033[0m";
}

} // namespace

ConsoleInterface::ConsoleInterface(SimulationEngine& e,
                                   std::istream& i,
                                   std::ostream& o,
                                   bool colorEnabled)
    : engine(e), in(i), out(o), useColor(colorEnabled) {}

void ConsoleInterface::run() {
    while (true) {
        printMenu();
        printStatusLine();

        int choice = 0;
        if (!InputValidator::readInt(in, out, "Choose option: ", 0, 12, choice)) {
            out << "\nInput closed. Goodbye.\n";
            return;
        }

        switch (choice) {
            case 0:
                out << "Goodbye.\n";
                return;
            case 1:  doStart(); break;
            case 2:  doPause(); break;
            case 3:  doResume(); break;
            case 4:  doReset(); break;
            case 5:  doStop(); break;
            case 6:  doAddVehicle(); break;
            case 7:  doToggleAutoGenerate(); break;
            case 8:  doConfigureDurations(); break;
            case 9:  doSignalMode(); break;
            case 10: displayStatus(); break;
            case 11: displayStatistics(); break;
            case 12: doStep(); break;
            default: break;
        }
    }
}

void ConsoleInterface::printMenu() const {
    out << "\n========== Traffic Management System ==========\n"
        << " 1. Start Simulation\n"
        << " 2. Pause Simulation\n"
        << " 3. Resume Simulation\n"
        << " 4. Reset Simulation\n"
        << " 5. Stop Simulation\n"
        << " 6. Add Vehicle\n"
        << " 7. Auto-generate Vehicles ON/OFF\n"
        << " 8. Configure Signal Durations\n"
        << " 9. Automatic/Manual Signal Mode\n"
        << "10. View Queues and Signals\n"
        << "11. View Statistics\n"
        << "12. Step N Ticks\n"
        << " 0. Quit\n"
        << "===============================================\n";
}

void ConsoleInterface::printStatusLine() const {
    out << paint("State: ", "\033[36m", useColor)
        << stateName(engine.getState())
        << " | Tick: " << engine.getCurrentTick()
        << " | Signals: " << (manualSignals ? "MANUAL" : "AUTOMATIC")
        << " | Auto-generation: ";

    if (autoGenerate) {
        out << "ON (p=" << generateProbability << ")";
    } else {
        out << "OFF";
    }

    out << "\n";
}

void ConsoleInterface::displayStatus() const {
    out << "\n--- Queues and Signals (Tick "
        << engine.getCurrentTick() << ") ---\n";

    out << std::left
        << std::setw(10) << "Approach"
        << std::setw(10) << "Signal"
        << "Waiting\n";

    for (Approach a : APPROACHES) {
        out << std::setw(10) << approachName(a)
            << std::setw(10)
            << signalName(engine.getSignalManager().getSignalState(a))
            << engine.getQueueManager().getQueueSize(a) << "\n";
    }

    out << "Total waiting: "
        << engine.getQueueManager().getTotalWaitingCount() << "\n"
        << "Total processed: "
        << engine.getStatsManager().getProcessedCount() << "\n";
}

void ConsoleInterface::displayStatistics() const {
    const int ticks = engine.getCurrentTick();
    const auto& stats = engine.getStatsManager();

    out << "\n========== Simulation Statistics ==========\n"
        << "Ticks elapsed:       " << ticks << "\n"
        << "Vehicles processed:  " << stats.getProcessedCount() << "\n"
        << "Total waiting time:  " << stats.getTotalWaitTime() << "\n"
        << "Average waiting time: "
        << std::fixed << std::setprecision(2)
        << stats.calculateAverageWaitTime() << "\n";

    if (ticks > 0) {
        out << "Throughput:           "
            << stats.calculateThroughput(ticks)
            << " vehicles/tick\n";
    } else {
        out << "Throughput:           0 vehicles/tick\n";
    }

    out << "Still waiting:        "
        << engine.getQueueManager().getTotalWaitingCount() << "\n";

    out << "\nBy approach (processed / waiting):\n";

    for (int i = 0; i < 4; ++i) {
        out << "  " << std::left << std::setw(8)
            << approachName(APPROACHES[i])
            << processedBy[i] << " / "
            << engine.getQueueManager().getQueueSize(APPROACHES[i])
            << "\n";
    }

    out << "===========================================\n";
}

void ConsoleInterface::printTickLine() {
    const int tick = engine.getCurrentTick() - 1;

    out << "t=" << std::right << std::setw(3) << tick << " |";

    for (Approach a : APPROACHES) {
        out << " " << approachName(a)[0] << ":"
            << std::setw(6)
            << signalName(engine.getSignalManager().getSignalState(a))
            << " q=" << std::setw(2)
            << engine.getQueueManager().getQueueSize(a) << " |";
    }

    out << " done=" << engine.getStatsManager().getProcessedCount()
        << "\n";

    for (const Vehicle& v : engine.getLastProcessed()) {
        out << "    -> Vehicle #" << v.id
            << " crossed from " << approachName(v.approach)
            << " (waited " << (tick - v.entryTick) << " ticks)\n";

        ++processedBy[static_cast<int>(v.approach)];
    }
}

bool ConsoleInterface::selectApproach(Approach& result) {
    out << "  1) North   2) South   3) East   4) West\n";

    int choice = 0;
    if (!InputValidator::readInt(in, out, "Select approach (1-4): ",
                                 1, 4, choice)) {
        return false;
    }

    result = APPROACHES[choice - 1];
    return true;
}

void ConsoleInterface::maybeGenerateVehicle() {
    if (!autoGenerate) return;

    Approach approach = Approach::NORTH;

    if (engine.getVehicleManager().generateVehicle(
            approach, generateProbability)) {
        Vehicle vehicle = engine.getVehicleManager().createVehicle(
            approach, engine.getCurrentTick());

        engine.getQueueManager().enqueueVehicle(vehicle);
    }
}

void ConsoleInterface::doStart() {
    if (engine.getState() == SimState::STOPPED) {
        engine.start();
        out << "Simulation started.\n";
    } else if (engine.getState() == SimState::RUNNING) {
        out << "Simulation is already running.\n";
    } else {
        out << "Simulation is paused. Use Resume to continue.\n";
    }
}

void ConsoleInterface::doPause() {
    if (engine.getState() != SimState::RUNNING) {
        out << "Cannot pause: simulation is not running.\n";
        return;
    }

    engine.pause();
    out << "Simulation paused at tick "
        << engine.getCurrentTick() << ".\n";
}

void ConsoleInterface::doResume() {
    if (engine.getState() != SimState::PAUSED) {
        out << "Cannot resume: simulation is not paused.\n";
        return;
    }

    engine.resume();
    out << "Simulation resumed.\n";
}

void ConsoleInterface::doReset() {
    engine.reset();

    manualSignals = false;
    for (int& count : processedBy) {
        count = 0;
    }

    out << "Simulation reset. Queues, counters, signals and clock cleared.\n";
}

void ConsoleInterface::doStop() {
    if (engine.getState() == SimState::STOPPED) {
        out << "Simulation is already stopped.\n";
        return;
    }

    engine.stop();
    out << "Simulation stopped. Final summary:\n";
    displayStatistics();
}

void ConsoleInterface::doAddVehicle() {
    Approach approach = Approach::NORTH;

    if (!selectApproach(approach)) return;

    int count = 0;
    if (!InputValidator::readInt(in, out,
                                 "How many vehicles (1-50): ",
                                 1, 50, count)) {
        return;
    }

    int firstId = 0;
    int lastId = 0;

    for (int i = 0; i < count; ++i) {
        Vehicle vehicle = engine.getVehicleManager().createVehicle(
            approach, engine.getCurrentTick());

        engine.getQueueManager().enqueueVehicle(vehicle);

        if (i == 0) firstId = vehicle.id;
        lastId = vehicle.id;
    }

    out << "Added " << count << " vehicle(s) (IDs "
        << firstId << "-" << lastId << ") to the "
        << approachName(approach) << " queue.\n"
        << "Vehicles waiting there: "
        << engine.getQueueManager().getQueueSize(approach) << "\n";
}

void ConsoleInterface::doToggleAutoGenerate() {
    if (autoGenerate) {
        autoGenerate = false;
        out << "Automatic vehicle generation disabled.\n";
        return;
    }

    double probability = 0.0;

    if (!InputValidator::readDouble(
            in, out, "Arrival probability per tick (0.01-1.00): ",
            0.01, 1.0, probability)) {
        return;
    }

    generateProbability = probability;
    autoGenerate = true;

    out << "Automatic vehicle generation enabled (p="
        << generateProbability << ").\n";
}

void ConsoleInterface::doConfigureDurations() {
    int green = 0;
    int yellow = 0;

    if (!InputValidator::readInt(
            in, out, "Green duration in ticks (1-60): ",
            1, 60, green)) {
        return;
    }

    if (!InputValidator::readInt(
            in, out, "Yellow duration in ticks (1-10): ",
            1, 10, yellow)) {
        return;
    }

    engine.getSignalManager().setDurations(green, yellow);

    out << "Signal durations set: green=" << green
        << ", yellow=" << yellow
        << " (applies from the next tick).\n";
}

void ConsoleInterface::doSignalMode() {
    int mode = 0;

    if (!InputValidator::readInt(
            in, out, "Signal mode (1=Automatic, 2=Manual): ",
            1, 2, mode)) {
        return;
    }

    if (mode == 1) {
        engine.getSignalManager().toggleManualMode(false);
        manualSignals = false;
        out << "Automatic signal cycle resumed.\n";
        return;
    }

    Approach approach = Approach::NORTH;

    out << "Which approach gets the green signal?\n";
    if (!selectApproach(approach)) return;

    engine.getSignalManager().toggleManualMode(true);
    engine.getSignalManager().setManualPhase(approach);
    manualSignals = true;

    out << "Manual mode: " << approachName(approach)
        << " is GREEN.\n";
}

void ConsoleInterface::doStep() {
    if (engine.getState() != SimState::RUNNING) {
        out << "Simulation is not running. Use Start or Resume first.\n";
        return;
    }

    int count = 0;

    if (!InputValidator::readInt(
            in, out, "Number of ticks (1-200): ",
            1, 200, count)) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        maybeGenerateVehicle();
        engine.tick();
        printTickLine();
    }
}
