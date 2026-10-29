#include "SimulationEngine.hpp"

namespace {
const Approach ALL_APPROACHES[] = {
    Approach::NORTH, Approach::SOUTH, Approach::EAST, Approach::WEST
};
} // namespace

void SimulationEngine::start() {
    if (state == SimState::STOPPED) state = SimState::RUNNING;
}

void SimulationEngine::pause() {
    if (state == SimState::RUNNING) state = SimState::PAUSED;
}

void SimulationEngine::resume() {
    if (state == SimState::PAUSED) state = SimState::RUNNING;
}

void SimulationEngine::stop() {
    state = SimState::STOPPED;   // tick count and statistics stay available for the summary
}

void SimulationEngine::reset() {
    currentTick = 0;
    state = SimState::STOPPED;
    lastProcessed.clear();
    queueMgr.reset();
    signalMgr.reset();
    statsMgr.reset();
    // autoGenEnabled / autoGenProbability are user settings and are kept
}

void SimulationEngine::setAutoGeneration(bool enabled, double probability) {
    if (probability < 0.0) probability = 0.0;
    if (probability > 1.0) probability = 1.0;
    autoGenEnabled = enabled;
    autoGenProbability = probability;
}

void SimulationEngine::generateVehicleIfDue() {
    if (!autoGenEnabled) return;
    Approach a = Approach::NORTH;
    if (vehicleMgr.generateVehicle(a, autoGenProbability)) {
        queueMgr.enqueueVehicle(vehicleMgr.createVehicle(a, currentTick));
    }
}

// SAD 4.2.2: only a GREEN approach may release its front vehicle.
// RED and YELLOW never dequeue; an empty queue is a no-op.
void SimulationEngine::processGreenApproaches() {
    for (Approach a : ALL_APPROACHES) {
        if (signalMgr.getSignalState(a) != SignalColor::GREEN) continue;
        Vehicle v{};
        if (queueMgr.dequeueVehicle(a, v)) {
            statsMgr.recordProcessedVehicle(v, currentTick);
            lastProcessed.push_back(v);
        }
    }
}

void SimulationEngine::tick() {
    if (state != SimState::RUNNING) return;

    lastProcessed.clear();
    generateVehicleIfDue();
    signalMgr.changeSignal(currentTick);
    processGreenApproaches();
    ++currentTick;
}

void SimulationEngine::runSimulation(int ticks) {
    if (ticks <= 0) return;
    if (state == SimState::STOPPED) start();
    for (int i = 0; i < ticks && state == SimState::RUNNING; ++i) {
        tick();
    }
}