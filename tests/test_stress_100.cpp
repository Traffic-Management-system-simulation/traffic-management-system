// TC-PERF-01 / TC-MEM-01: 100-vehicle stress run (TMS-NF-001, TMS-NF-007, TMS-SR-005)
#include <chrono>
#include <iostream>
#include <set>
#include "SimulationEngine.hpp"

namespace {
int failures = 0;
const Approach kApps[] = {Approach::NORTH, Approach::SOUTH, Approach::EAST, Approach::WEST};
const int kMaxTicks = 2000;
const double kTimeLimitMs = 1000.0;   // TODO: set to the limit in TC-PERF-01

void check(bool ok, const char* name) {
    std::cout << (ok ? "PASS: " : "FAIL: ") << name << "\n";
    if (!ok) ++failures;
}

bool green(SimulationEngine& e, Approach a) {
    return e.getSignalManager().getSignalState(a) == SignalColor::GREEN;
}

bool conflict(SimulationEngine& e) {
    bool ns = green(e, Approach::NORTH) || green(e, Approach::SOUTH);
    bool ew = green(e, Approach::EAST) || green(e, Approach::WEST);
    return ns && ew;
}

void addVehicles(SimulationEngine& e, Approach a, int n) {
    for (int i = 0; i < n; ++i)
        e.getQueueManager().enqueueVehicle(
            e.getVehicleManager().createVehicle(a, e.getCurrentTick()));
}

// Ticks until all queues are empty. Returns ticks used; records safety/ID results.
int drain(SimulationEngine& e, std::set<int>& ids, bool& conflictSeen, bool& dupSeen) {
    int used = 0;
    while (e.getQueueManager().getTotalWaitingCount() > 0 && used < kMaxTicks) {
        e.tick();
        ++used;
        if (conflict(e)) conflictSeen = true;
        for (const Vehicle& v : e.getLastProcessed())
            if (!ids.insert(v.id).second) dupSeen = true;
    }
    return used;
}
}  // namespace

int main() {
    // ---- Test 1: 100 manually inserted vehicles, 25 per approach ----
    {
        SimulationEngine e;
        e.reset();
        for (Approach a : kApps) addVehicles(e, a, 25);
        check(e.getQueueManager().getTotalWaitingCount() == 100, "100 vehicles queued");

        auto t0 = std::chrono::steady_clock::now();
        e.start();
        std::set<int> ids; bool conf = false, dup = false;
        int ticks = drain(e, ids, conf, dup);
        double ms = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - t0).count();

        std::cout << "  drained in " << ticks << " ticks, " << ms << " ms\n";
        check(e.getStatsManager().getProcessedCount() == 100, "all 100 vehicles processed");
        check(e.getQueueManager().getTotalWaitingCount() == 0, "all queues empty");
        check(ids.size() == 100 && !dup, "100 unique vehicle IDs");
        check(!conf, "no conflicting greens during run");
        check(ticks < kMaxTicks, "finished within tick budget");
        check(ms < kTimeLimitMs, "run time within limit");
        check(e.getStatsManager().calculateAverageWaitTime() >= 0.0, "average wait is valid");
    }

    // ---- Test 2: 100 auto-generated vehicles (p = 1.0), one arrival per tick ----
    {
        SimulationEngine e;
        e.reset();
        e.start();
        int generated = 0;
        for (int i = 0; i < 100; ++i) {
            Approach a = Approach::NORTH;
            if (e.getVehicleManager().generateVehicle(a, 1.0)) {
                e.getQueueManager().enqueueVehicle(
                    e.getVehicleManager().createVehicle(a, e.getCurrentTick()));
                ++generated;
            }
            e.tick();
        }
        std::set<int> ids; bool conf = false, dup = false;
        drain(e, ids, conf, dup);
        check(generated == 100, "auto-generation produced 100 vehicles");
        check(e.getStatsManager().getProcessedCount() == 100, "auto-generated vehicles all processed");
    }

    // ---- Test 3: stop and reset with vehicles still queued ----
    {
        SimulationEngine e;
        e.reset();
        for (Approach a : kApps) addVehicles(e, a, 25);
        e.start();
        for (int i = 0; i < 20; ++i) e.tick();
        e.stop();
        check(e.getQueueManager().getTotalWaitingCount() > 0, "vehicles still queued after Stop");
        e.reset();
        check(e.getQueueManager().getTotalWaitingCount() == 0, "Reset clears queues");
        check(e.getStatsManager().getProcessedCount() == 0, "Reset clears statistics");
        check(e.getCurrentTick() == 0, "Reset clears clock");
    }

    // ---- Test 4: repeated load/reset cycles (leak hunting) ----
    {
        SimulationEngine e;
        for (int round = 0; round < 50; ++round) {
            e.reset();
            for (Approach a : kApps) addVehicles(e, a, 25);
            e.start();
            for (int i = 0; i < 30; ++i) e.tick();
        }
        e.reset();
        check(e.getQueueManager().getTotalWaitingCount() == 0, "50 load/reset cycles survived");
    }

    std::cout << (failures == 0 ? "All stress tests passed.\n" : "Some stress tests FAILED.\n");
    return failures == 0 ? 0 : 1;
}