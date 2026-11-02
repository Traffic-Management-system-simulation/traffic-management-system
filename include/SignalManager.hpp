#ifndef SIGNAL_MANAGER_HPP
#define SIGNAL_MANAGER_HPP

#include <map>
#include "Common.hpp"

class SignalManager {
private:
    std::map<Approach, SignalColor> signals;
    int greenDuration = 10;
    int yellowDuration = 3;
    bool isManualMode = false;
public:
    SignalManager() {};
    SignalColor getSignalState(Approach app) const;
    void changeSignal(int tick);
    void setManualPhase(Approach activeGreen);
    void setDurations(int green, int yellow);
    int getGreenDuration() const;
    int getYellowDuration() const;
    void toggleManualMode(bool enable);
    void reset();
};

#endif // SIGNAL_MANAGER_HPP
