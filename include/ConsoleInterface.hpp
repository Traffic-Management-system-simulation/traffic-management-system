
#ifndef CONSOLE_INTERFACE_HPP
#define CONSOLE_INTERFACE_HPP

#include <iosfwd>
#include "SimulationEngine.hpp"

class ConsoleInterface {
public:
    ConsoleInterface(SimulationEngine& engine,
                     std::istream& in,
                     std::ostream& out,
                     bool colorEnabled = false);

    void run();

private:
    SimulationEngine& engine;
    std::istream& in;
    std::ostream& out;

    bool useColor;
    bool autoGenerate = false;
    bool manualSignals = false;
    double generateProbability = 0.30;

    int processedBy[4] = {0, 0, 0, 0};

    void printMenu() const;
    void printStatusLine() const;
    void displayStatus() const;
    void displayStatistics() const;
    void printTickLine();

    bool selectApproach(Approach& result);
    void maybeGenerateVehicle();

    void doStart();
    void doPause();
    void doResume();
    void doReset();
    void doStop();
    void doAddVehicle();
    void doToggleAutoGenerate();
    void doConfigureDurations();
    void doSignalMode();
    void doStep();
    void doSaveSettings();
    void doLoadSettings();
    void doExportStatistics();
};

#endif
