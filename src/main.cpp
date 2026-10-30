// Console Interface / Controller (SAD 3.4, 4.5).
// Menu-driven front end for the SimulationEngine. All traffic logic lives in
// the managers; this file only reads input, validates it and prints state.
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include "SimulationEngine.hpp"

namespace {

bool useColor = false;   // enabled with --color (used for the demo recording)

std::string paint(const std::string& text, const char* code) {
    return useColor ? std::string(code) + text + "\033[0m" : text;
}

const char* approachName(Approach a) {
    switch (a) {
        case Approach::NORTH: return "NORTH";
        case Approach::SOUTH: return "SOUTH";
        case Approach::EAST:  return "EAST";
        case Approach::WEST:  return "WEST";
    }
    return "?";
}

std::string signalText(SignalColor c) {
    switch (c) {
        case SignalColor::GREEN:  return paint("GREEN ", "\033[1;32m");
        case SignalColor::YELLOW: return paint("YELLOW", "\033[1;33m");
        case SignalColor::RED:    return paint("RED   ", "\033[1;31m");
    }
    return "?";
}

const char* stateName(SimState s) {
    switch (s) {
        case SimState::STOPPED: return "STOPPED";
        case SimState::RUNNING: return "RUNNING";
        case SimState::PAUSED:  return "PAUSED";
    }
    return "?";
}

const Approach kApproaches[] = {Approach::NORTH, Approach::SOUTH,
                                Approach::EAST, Approach::WEST};

// ---------- input helpers: never crash on bad input (TMS-F-021, SR-001/004) ----------

bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt << std::flush;
    return static_cast<bool>(std::getline(std::cin, out));   // false on EOF
}

bool parseInt(const std::string& text, int lo, int hi, int& out) {
    const char* begin = text.c_str();
    char* end = nullptr;
    errno = 0;
    long v = std::strtol(begin, &end, 10);
    if (end == begin || errno != 0) return false;
    while (*end == ' ' || *end == '\t' || *end == '\r') ++end;   // allow trailing blanks
    if (*end != '\0' || v < lo || v > hi) return false;
    out = static_cast<int>(v);
    return true;
}

bool parseDouble(const std::string& text, double lo, double hi, double& out) {
    const char* begin = text.c_str();
    char* end = nullptr;
    errno = 0;
    double v = std::strtod(begin, &end);
    if (end == begin || errno != 0) return false;
    while (*end == ' ' || *end == '\t' || *end == '\r') ++end;
    if (*end != '\0' || !(v >= lo && v <= hi)) return false;
    out = v;
    return true;
}

// Re-prompts until valid. Returns false only when input ends (EOF).
bool askInt(const std::string& prompt, int lo, int hi, int& out) {
    std::string line;
    while (readLine(prompt, line)) {
        if (parseInt(line, lo, hi, out)) return true;
        std::cout << paint("  Invalid input", "\033[1;31m") << ": enter a whole number from "
                  << lo << " to " << hi << ".\n";
    }
    return false;
}

bool askDouble(const std::string& prompt, double lo, double hi, double& out) {
    std::string line;
    while (readLine(prompt, line)) {
        if (parseDouble(line, lo, hi, out)) return true;
        std::cout << paint("  Invalid input", "\033[1;31m") << ": enter a number from "
                  << lo << " to " << hi << ".\n";
    }
    return false;
}

bool askApproach(int& index) {
    return askInt("  Approach (1=North 2=South 3=East 4=West): ", 1, 4, index);
}

// ---------- console session state ----------

struct Session {
    SimulationEngine engine;
    bool autoGenerate = false;
    double generateProbability = 0.30;
    bool manualSignals = false;
    int processedBy[4] = {0, 0, 0, 0};
};

void printMenu() {
    std::cout << "\n"
              << " [1] Start   [2] Pause   [3] Resume   [4] Reset   [5] Stop   [6] Add vehicle\n"
              << " [7] Auto-generate on/off   [8] Signal durations   [9] Auto/Manual signals\n"
              << " [10] View queues & signals   [11] Statistics   [12] Step N ticks   [0] Quit\n";
}

void printStatusLine(const Session& s) {
    std::cout << paint("State: ", "\033[36m") << stateName(s.engine.getState())
              << " | Tick " << s.engine.getCurrentTick()
              << " | Signals: " << (s.manualSignals ? "MANUAL" : "AUTO")
              << " | Auto-generate: ";
    if (s.autoGenerate) std::cout << "ON (p=" << s.generateProbability << ")";
    else std::cout << "OFF";
    std::cout << "\n";
}

void printView(Session& s) {
    std::cout << paint("--- Queues & signals after ", "\033[36m")
              << paint(std::to_string(s.engine.getCurrentTick()) + " tick(s) ---", "\033[36m") << "\n";
    for (Approach a : kApproaches) {
        std::cout << "  " << std::left << std::setw(6) << approachName(a) << " "
                  << signalText(s.engine.getSignalManager().getSignalState(a))
                  << "  waiting: " << s.engine.getQueueManager().getQueueSize(a) << "\n";
    }
    std::cout << "  Total waiting: " << s.engine.getQueueManager().getTotalWaitingCount()
              << " | Processed: " << s.engine.getStatsManager().getProcessedCount() << "\n";
}

void printStatistics(Session& s) {
    const int ticks = s.engine.getCurrentTick();
    const auto& stats = s.engine.getStatsManager();
    const auto& queues = s.engine.getQueueManager();

    std::cout << "\n========== SIMULATION SUMMARY ==========\n";

    std::cout << std::left << std::setw(28) << "Metric"
              << "Value\n";
    std::cout << "-----------------------------------------\n";

    std::cout << std::left << std::setw(28) << "Total simulation ticks"
              << ticks << "\n";

    std::cout << std::left << std::setw(28) << "Vehicles processed"
              << stats.getProcessedCount() << "\n";

    std::cout << std::left << std::setw(28) << "Vehicles still waiting"
              << queues.getTotalWaitingCount() << "\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << std::left << std::setw(28) << "Average waiting time"
              << stats.calculateAverageWaitTime() << " ticks\n";

    std::cout << std::left << std::setw(28) << "Throughput"
              << stats.calculateThroughput(ticks) << " vehicles/tick\n";

    std::cout << "\n";
    std::cout << std::left << std::setw(12) << "Approach"
              << std::setw(15) << "Processed"
              << "Waiting\n";

    std::cout << "-----------------------------------------\n";

    for (int i = 0; i < 4; ++i) {
        std::cout << std::left << std::setw(12)
                  << approachName(kApproaches[i])
                  << std::setw(15) << s.processedBy[i]
                  << queues.getQueueSize(kApproaches[i]) << "\n";
    }

    std::cout << "=========================================\n";
}

// One line per tick (SRS 3.1) plus one line per vehicle that crossed.
void printTickLine(Session& s) {
    const int t = s.engine.getCurrentTick() - 1;   // the tick that was just processed
    std::cout << "t=" << std::right << std::setw(3) << t << " |";
    for (Approach a : kApproaches) {
        std::cout << " " << std::string(1, approachName(a)[0]) << ":"
                  << signalText(s.engine.getSignalManager().getSignalState(a))
                  << " q=" << std::left << std::setw(2)
                  << s.engine.getQueueManager().getQueueSize(a) << "|";
    }
    std::cout << " done=" << s.engine.getStatsManager().getProcessedCount() << "\n";
    for (const Vehicle& v : s.engine.getLastProcessed()) {
        std::cout << "        " << paint("-> Vehicle #" + std::to_string(v.id) + " crossed from "
                                         + approachName(v.approach), "\033[32m")
                  << " (waited " << (t - v.entryTick) << " ticks)\n";
        s.processedBy[static_cast<int>(v.approach)]++;
    }
}

// ---------- menu actions ----------

void doAddVehicle(Session& s) {
    int a = 0, count = 0;
    if (!askApproach(a) || !askInt("  How many vehicles (1-50): ", 1, 50, count)) return;
    Approach app = kApproaches[a - 1];
    int firstId = 0, lastId = 0;
    for (int i = 0; i < count; ++i) {
        Vehicle v = s.engine.getVehicleManager().createVehicle(app, s.engine.getCurrentTick());
        s.engine.getQueueManager().enqueueVehicle(v);
        if (i == 0) firstId = v.id;
        lastId = v.id;
    }
    std::cout << "  Added " << count << " vehicle(s) (IDs " << firstId << "-" << lastId
              << ") to the " << approachName(app) << " queue. Waiting there now: "
              << s.engine.getQueueManager().getQueueSize(app) << "\n";
}

void doToggleAutoGenerate(Session& s) {
    if (s.autoGenerate) {
        s.autoGenerate = false;
        std::cout << "  Auto-generation OFF.\n";
        return;
    }
    double p = 0.0;
    if (!askDouble("  Arrival probability per tick (0.01-1.00): ", 0.01, 1.0, p)) return;
    s.generateProbability = p;
    s.autoGenerate = true;
    std::cout << "  Auto-generation ON (p=" << p << ").\n";
}

void doDurations(Session& s) {
    int g = 0, y = 0;
    if (!askInt("  Green duration in ticks (1-60): ", 1, 60, g) ||
        !askInt("  Yellow duration in ticks (1-10): ", 1, 10, y)) return;
    s.engine.getSignalManager().setDurations(g, y);
    std::cout << "  Durations set: green=" << g << ", yellow=" << y << " (applies from the next tick).\n";
}

void doSignalMode(Session& s) {
    int mode = 0;
    if (!askInt("  Signal mode (1=Automatic 2=Manual): ", 1, 2, mode)) return;
    if (mode == 1) {
        s.engine.getSignalManager().toggleManualMode(false);
        s.manualSignals = false;
        std::cout << "  Automatic signal cycle resumed.\n";
        return;
    }
    int a = 0;
    if (!askApproach(a)) return;
    s.engine.getSignalManager().toggleManualMode(true);
    s.engine.getSignalManager().setManualPhase(kApproaches[a - 1]);
    s.manualSignals = true;
    std::cout << "  Manual mode: only " << approachName(kApproaches[a - 1]) << " is GREEN.\n";
}

void doStep(Session& s) {
    if (s.engine.getState() != SimState::RUNNING) {
        std::cout << "  Simulation is not running (state: " << stateName(s.engine.getState())
                  << "). Use Start or Resume first.\n";
        return;
    }
    int n = 0;
    if (!askInt("  Number of ticks (1-200): ", 1, 200, n)) return;
    for (int i = 0; i < n; ++i) {
        if (s.autoGenerate) {
            Approach a = Approach::NORTH;
            if (s.engine.getVehicleManager().generateVehicle(a, s.generateProbability)) {
                s.engine.getQueueManager().enqueueVehicle(
                    s.engine.getVehicleManager().createVehicle(a, s.engine.getCurrentTick()));
            }
        }
        s.engine.tick();
        printTickLine(s);
    }
}

void doStart(Session& s) {
    switch (s.engine.getState()) {
        case SimState::STOPPED: s.engine.start();
            std::cout << "  Simulation started.\n"; break;
        case SimState::RUNNING: std::cout << "  Already running.\n"; break;
        case SimState::PAUSED:  std::cout << "  Simulation is paused - use Resume.\n"; break;
    }
}

void doPause(Session& s) {
    if (s.engine.getState() != SimState::RUNNING) { std::cout << "  Nothing to pause (not running).\n"; return; }
    s.engine.pause();
    std::cout << "  Simulation paused. Clock and queues are frozen.\n";
}

void doResume(Session& s) {
    if (s.engine.getState() != SimState::PAUSED) { std::cout << "  Nothing to resume (not paused).\n"; return; }
    s.engine.resume();
    std::cout << "  Simulation resumed.\n";
}

void doStop(Session& s) {
    if (s.engine.getState() == SimState::STOPPED) { std::cout << "  Already stopped.\n"; return; }
    s.engine.stop();
    std::cout << "  Simulation stopped. Final summary:";
    printStatistics(s);
}

void doReset(Session& s) {
    s.engine.reset();
    s.manualSignals = false;
    for (int& n : s.processedBy) n = 0;
    std::cout << "  Reset: queues, counters, signals and clock are back to their initial state.\n";
}

}  // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--color") == 0) useColor = true;
    }

    std::cout << "--- Traffic Management System Simulation ---" << std::endl;
    Session session;
    session.engine.reset();   // puts the signals into their initial phase

    int choice = -1;
    while (true) {
        printMenu();
        printStatusLine(session);
        if (!askInt("Choose option: ", 0, 12, choice)) break;   // EOF ends the session cleanly
        switch (choice) {
            case 1:  doStart(session); break;
            case 2:  doPause(session); break;
            case 3:  doResume(session); break;
            case 4:  doReset(session); break;
            case 5:  doStop(session); break;
            case 6:  doAddVehicle(session); break;
            case 7:  doToggleAutoGenerate(session); break;
            case 8:  doDurations(session); break;
            case 9:  doSignalMode(session); break;
            case 10: printView(session); break;
            case 11: printStatistics(session); break;
            case 12: doStep(session); break;
            case 0:  std::cout << "Goodbye.\n"; return 0;
            default: break;
        }
    }
    std::cout << "\nInput closed. Goodbye.\n";
    return 0;
}