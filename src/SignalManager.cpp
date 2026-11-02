#include "SignalManager.hpp"

SignalColor SignalManager::getSignalState(Approach app) const {
    auto it = signals.find(app);
    return it == signals.end() ? SignalColor::RED : it->second;
}

// Sets one axis (North-South or East-West) to the given colour and
// forces the other axis to RED. Opposing axes are never green together.
static void setAxis(std::map<Approach, SignalColor>& signals,
                    bool northSouth, SignalColor colour) {
    const SignalColor ns = northSouth ? colour : SignalColor::RED;
    const SignalColor ew = northSouth ? SignalColor::RED : colour;
    signals[Approach::NORTH] = ns;
    signals[Approach::SOUTH] = ns;
    signals[Approach::EAST] = ew;
    signals[Approach::WEST] = ew;
}

void SignalManager::changeSignal(int tick) {
    if (isManualMode || tick < 0) return;

    const int green = greenDuration;
    const int yellow = yellowDuration;
    const int cycle = 2 * (green + yellow);
    const int phase = tick % cycle;

    // Phase 1: North-South green, then yellow.
    // Phase 2: East-West green, then yellow.
    if (phase < green) {
        setAxis(signals, true, SignalColor::GREEN);
    } else if (phase < green + yellow) {
        setAxis(signals, true, SignalColor::YELLOW);
    } else if (phase < 2 * green + yellow) {
        setAxis(signals, false, SignalColor::GREEN);
    } else {
        setAxis(signals, false, SignalColor::YELLOW);
    }
}

void SignalManager::setManualPhase(Approach activeGreen) {
    for (auto app : {Approach::NORTH, Approach::SOUTH,
                     Approach::EAST, Approach::WEST}) {
        signals[app] =
            (app == activeGreen) ? SignalColor::GREEN : SignalColor::RED;
    }
}

void SignalManager::setDurations(int green, int yellow) {
    if (green > 0) greenDuration = green;
    if (yellow > 0) yellowDuration = yellow;
}
int SignalManager::getGreenDuration() const {
    return greenDuration;
}

int SignalManager::getYellowDuration() const {
    return yellowDuration;
}

void SignalManager::toggleManualMode(bool enable) {
    isManualMode = enable;
}

void SignalManager::reset() {
    isManualMode = false;
    setAxis(signals, true, SignalColor::GREEN);
}