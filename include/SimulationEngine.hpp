#ifndef SIMULATION_ENGINE_HPP
#define SIMULATION_ENGINE_HPP

#include <vector>
#include "QueueManager.hpp"
#include "SignalManager.hpp"
#include "StatisticsManager.hpp"
#include "Vehicle.hpp"

enum class SimState { STOPPED, RUNNING, PAUSED };

class SimulationEngine {
private:
    int currentTick = 0;
    SimState state = SimState::STOPPED;
    bool autoGenEnabled = false;
    double autoGenProbability = 0.0;
    std::vector<Vehicle> lastProcessed;   // vehicles released by the most recent tick
    QueueManager queueMgr;
    SignalManager signalMgr;
    StatisticsManager statsMgr;
    VehicleManager vehicleMgr;

    void generateVehicleIfDue();
    void processGreenApproaches();

public:
    // Runs up to `ticks` ticks. Starts the simulation first if it is STOPPED.
    // Stops early if the state leaves RUNNING.
    void runSimulation(int ticks);

    // Advances one tick. Does nothing unless the state is RUNNING.
    void tick();

    void start();    // STOPPED -> RUNNING
    void pause();    // RUNNING -> PAUSED
    void resume();   // PAUSED  -> RUNNING
    void reset();    // back to tick 0, STOPPED, all modules cleared
    void stop();     // any     -> STOPPED (tick count and statistics are kept)

    void setAutoGeneration(bool enabled, double probability);
    bool isAutoGenerationEnabled() const { return autoGenEnabled; }

    // Vehicles that crossed during the most recent executed tick (empty if none).
    const std::vector<Vehicle>& getLastProcessed() const { return lastProcessed; }

    int getCurrentTick() const { return currentTick; }
    SimState getState() const { return state; }

    QueueManager& getQueueManager() { return queueMgr; }
    SignalManager& getSignalManager() { return signalMgr; }
    StatisticsManager& getStatsManager() { return statsMgr; }
    VehicleManager& getVehicleManager() { return vehicleMgr; }
};

#endif // SIMULATION_ENGINE_HPP