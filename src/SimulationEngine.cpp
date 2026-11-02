#include "SimulationEngine.hpp"

namespace {
const Approach kApproaches[] = {Approach::NORTH, Approach::SOUTH,
                                Approach::EAST, Approach::WEST};
}

// One discrete time step (TMS-F-016, F-017, F-015):
//   1. ask the signal manager what colour each approach shows at this tick
//   2. for every approach that is GREEN, dequeue its front vehicle (if any)
//   3. record the vehicle in the statistics, then advance the clock
// Red and yellow approaches are never dequeued, so their vehicles keep waiting.
void SimulationEngine::tick() {
    if (state != SimState::RUNNING) return;

    lastProcessed.clear();
    signalMgr.changeSignal(currentTick);

        if (autoGenerate) {
        Approach approach = Approach::NORTH;
        if (vehicleMgr.generateVehicle(approach, generateProbability)) {
            queueMgr.enqueueVehicle(
                vehicleMgr.createVehicle(approach, currentTick));
        }
    }

    for (Approach app : kApproaches) {
        if (signalMgr.getSignalState(app) != SignalColor::GREEN) continue;

        Vehicle v{};
        if (queueMgr.dequeueVehicle(app, v)) {   // false on empty queue: safe
            statsMgr.recordProcessedVehicle(v, currentTick);
            lastProcessed.push_back(v);
        }
    }
    ++currentTick;
}

void SimulationEngine::start()  { if (state == SimState::STOPPED) state = SimState::RUNNING; }
void SimulationEngine::pause()  { if (state == SimState::RUNNING) state = SimState::PAUSED;  }
void SimulationEngine::resume() { if (state == SimState::PAUSED)  state = SimState::RUNNING; }
void SimulationEngine::stop()   { state = SimState::STOPPED; }   // statistics are kept for the summary

// TMS-F-020: back to the initial state (queues, counters, signals, clock).
void SimulationEngine::reset() {
    state = SimState::STOPPED;
    currentTick = 0;
    queueMgr.reset();
    statsMgr.reset();
    signalMgr.reset();
    lastProcessed.clear();
    autoGenerate = false;
    generateProbability = 0.30;
}

// Headless run used by tests/automation: run `ticks` steps without any console I/O.
void SimulationEngine::runSimulation(int ticks) {
    start();
    for (int i = 0; i < ticks && state == SimState::RUNNING; ++i) tick();
}