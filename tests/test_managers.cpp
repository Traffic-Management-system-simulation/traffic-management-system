#include <iostream>
#include "../include/Vehicle.hpp"
#include "../include/QueueManager.hpp"
#include "../include/SignalManager.hpp"
#include "../include/Common.hpp"

static int passed = 0;
static int failed = 0;

#define CHECK(condition, message) do { \
    if (condition) { \
        ++passed; \
        std::cout << "PASS: " << message << '\n'; \
    } else { \
        ++failed; \
        std::cout << "FAIL: " << message << '\n'; \
    } \
} while (false)

int main() {
    std::cout << "=== TC-VM: Vehicle Tests ===\n";

    VehicleManager vm;
    Vehicle v1 = vm.createVehicle(Approach::NORTH, 0);
    Vehicle v2 = vm.createVehicle(Approach::SOUTH, 2);

    CHECK(v1.id != v2.id, "Vehicle IDs are unique");
    CHECK(v1.entryTick == 0, "First vehicle entry tick");
    CHECK(v2.entryTick == 2, "Second vehicle entry tick");
    CHECK(v1.approach == Approach::NORTH,
          "Vehicle approach is stored");

    std::cout << "\n=== TC-Q: Queue Tests ===\n";

    QueueManager qm;
    qm.enqueueVehicle(v1);
    qm.enqueueVehicle(v2);

    CHECK(qm.getQueueSize(Approach::NORTH) == 1,
          "North queue size");
    CHECK(qm.getQueueSize(Approach::SOUTH) == 1,
          "South queue size");
    CHECK(qm.getTotalWaitingCount() == 2,
          "Total waiting count");

    Vehicle out{};
    CHECK(qm.dequeueVehicle(Approach::NORTH, out),
          "Dequeue from non-empty queue");
    CHECK(out.id == v1.id, "FIFO order");
    CHECK(!qm.dequeueVehicle(Approach::NORTH, out),
          "Empty queue returns false");

    qm.reset();
    CHECK(qm.getTotalWaitingCount() == 0,
          "Queue reset clears vehicles");

    std::cout << "\n=== TC-SIG: Signal Tests ===\n";

    SignalManager sm;
    sm.reset();
    sm.setDurations(2, 1);

    // Safety: opposing axes must never both show GREEN or YELLOW.
    auto isSafe = [](const SignalManager& s) {
        auto active = [&s](Approach a) {
            return s.getSignalState(a) != SignalColor::RED;
        };
        bool northSouth = active(Approach::NORTH) || active(Approach::SOUTH);
        bool eastWest = active(Approach::EAST) || active(Approach::WEST);
        return !(northSouth && eastWest);
    };

    // TC-SIG-01: displayed state of every approach matches the controller.
    sm.changeSignal(0);
    CHECK(sm.getSignalState(Approach::NORTH) == SignalColor::GREEN &&
          sm.getSignalState(Approach::SOUTH) == SignalColor::GREEN &&
          sm.getSignalState(Approach::EAST) == SignalColor::RED &&
          sm.getSignalState(Approach::WEST) == SignalColor::RED,
          "TC-SIG-01: Approach states at tick 0 (N+S green, E+W red)");

    // TC-SIG-02: automatic mode follows the predefined safe sequence.
    sm.changeSignal(2);
    CHECK(sm.getSignalState(Approach::NORTH) == SignalColor::YELLOW &&
          sm.getSignalState(Approach::SOUTH) == SignalColor::YELLOW,
          "TC-SIG-02: North-South yellow after green duration");
    sm.changeSignal(3);
    CHECK(sm.getSignalState(Approach::EAST) == SignalColor::GREEN &&
          sm.getSignalState(Approach::WEST) == SignalColor::GREEN,
          "TC-SIG-02: East-West green after North-South yellow");
    sm.changeSignal(5);
    CHECK(sm.getSignalState(Approach::EAST) == SignalColor::YELLOW &&
          sm.getSignalState(Approach::WEST) == SignalColor::YELLOW,
          "TC-SIG-02: East-West yellow after green duration");
    sm.changeSignal(6);
    CHECK(sm.getSignalState(Approach::NORTH) == SignalColor::GREEN,
          "TC-SIG-02: Cycle returns to North-South");

    // TC-SIG-03: changed durations apply to the next phase.
    // With green=3 and yellow=2: green ticks 0-2, yellow ticks 3-4,
    // East-West green from tick 5.
    SignalManager sd;
    sd.reset();
    sd.setDurations(3, 2);
    sd.changeSignal(2);
    CHECK(sd.getSignalState(Approach::NORTH) == SignalColor::GREEN,
          "TC-SIG-03: Longer green keeps North green at tick 2");
    sd.changeSignal(3);
    CHECK(sd.getSignalState(Approach::NORTH) == SignalColor::YELLOW,
          "TC-SIG-03: North yellow after new green duration");
    sd.changeSignal(5);
    CHECK(sd.getSignalState(Approach::EAST) == SignalColor::GREEN,
          "TC-SIG-03: East green after new yellow duration");
    sd.setDurations(0, -1);
    sd.changeSignal(5);
    CHECK(sd.getSignalState(Approach::EAST) == SignalColor::GREEN,
          "TC-SIG-03: Invalid durations are ignored");

    // TC-SIG-04: manual selection is applied and not overridden.
    sm.toggleManualMode(true);
    sm.setManualPhase(Approach::WEST);
    sm.changeSignal(0);
    CHECK(sm.getSignalState(Approach::WEST) == SignalColor::GREEN &&
          sm.getSignalState(Approach::NORTH) == SignalColor::RED &&
          sm.getSignalState(Approach::SOUTH) == SignalColor::RED &&
          sm.getSignalState(Approach::EAST) == SignalColor::RED,
          "TC-SIG-04: Manual phase applied and automatic cycle paused");

    sm.toggleManualMode(false);
    sm.changeSignal(0);
    CHECK(sm.getSignalState(Approach::NORTH) == SignalColor::GREEN,
          "TC-SIG-04: Automatic mode resumes after manual mode");

    // TC-SAFE-01: no conflicting signals at any tick over several cycles,
    // with default and changed durations, and in manual mode.
    bool allSafe = true;
    SignalManager sf;
    sf.reset();
    for (int t = 0; t < 100; ++t) {
        sf.changeSignal(t);
        if (!isSafe(sf)) allSafe = false;
    }
    sf.setDurations(4, 2);
    for (int t = 0; t < 100; ++t) {
        sf.changeSignal(t);
        if (!isSafe(sf)) allSafe = false;
    }
    sf.toggleManualMode(true);
    for (Approach a : {Approach::NORTH, Approach::SOUTH,
                       Approach::EAST, Approach::WEST}) {
        sf.setManualPhase(a);
        if (!isSafe(sf)) allSafe = false;
    }
    CHECK(allSafe, "TC-SAFE-01: No conflicting signals over 200 ticks and manual mode");

    std::cout << "\nPassed: " << passed
              << " | Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}