#include "SimulationEngine.hpp"
#include <iostream>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << "FAIL: " #cond " (line " << __LINE__ << ")\n";    \
            ++failures;                                                    \
        }                                                                  \
    } while (0)

static void addVehicle(SimulationEngine& e, Approach a) {
    e.getQueueManager().enqueueVehicle(
        e.getVehicleManager().createVehicle(a, e.getCurrentTick()));
}

static void northGreen(SimulationEngine& e) {
    e.getSignalManager().toggleManualMode(true);
    e.getSignalManager().setManualPhase(Approach::NORTH);
}

void test_initial_state() {
    SimulationEngine e;
    CHECK(e.getState() == SimState::STOPPED);
    CHECK(e.getCurrentTick() == 0);
}

void test_tick_ignored_when_stopped() {
    SimulationEngine e;
    e.tick();
    CHECK(e.getCurrentTick() == 0);
}

void test_start_and_tick() {
    SimulationEngine e;
    e.start();
    CHECK(e.getState() == SimState::RUNNING);
    e.tick();
    e.tick();
    CHECK(e.getCurrentTick() == 2);
}

void test_pause_resume() {
    SimulationEngine e;
    e.start();
    e.tick();
    e.pause();
    CHECK(e.getState() == SimState::PAUSED);
    e.tick();
    CHECK(e.getCurrentTick() == 1);          // paused: no progress
    e.resume();
    CHECK(e.getState() == SimState::RUNNING);
    e.tick();
    CHECK(e.getCurrentTick() == 2);
}

void test_invalid_transitions_are_ignored() {
    SimulationEngine e;
    e.pause();
    CHECK(e.getState() == SimState::STOPPED);
    e.resume();
    CHECK(e.getState() == SimState::STOPPED);
    e.start();
    e.resume();                               // not paused
    CHECK(e.getState() == SimState::RUNNING);
    e.start();                                // already running
    CHECK(e.getState() == SimState::RUNNING);
}

void test_stop_keeps_tick_and_stats() {
    SimulationEngine e;
    northGreen(e);
    addVehicle(e, Approach::NORTH);
    e.start();
    e.tick();
    e.stop();
    CHECK(e.getState() == SimState::STOPPED);
    CHECK(e.getCurrentTick() == 1);
    CHECK(e.getStatsManager().getProcessedCount() == 1);
    e.tick();
    CHECK(e.getCurrentTick() == 1);          // stopped: no progress
}

void test_green_releases_one_vehicle_per_tick() {
    SimulationEngine e;
    northGreen(e);
    for (int i = 0; i < 3; ++i) addVehicle(e, Approach::NORTH);
    addVehicle(e, Approach::EAST);
    e.start();
    e.tick();
    e.tick();
    CHECK(e.getQueueManager().getQueueSize(Approach::NORTH) == 1);
    CHECK(e.getQueueManager().getQueueSize(Approach::EAST) == 1);   // red: untouched
    CHECK(e.getStatsManager().getProcessedCount() == 2);
}

void test_red_approach_never_dequeues() {
    SimulationEngine e;
    northGreen(e);
    addVehicle(e, Approach::SOUTH);
    e.start();
    for (int i = 0; i < 5; ++i) e.tick();
    CHECK(e.getQueueManager().getQueueSize(Approach::SOUTH) == 1);
    CHECK(e.getStatsManager().getProcessedCount() == 0);
}

void test_empty_queue_on_green_is_safe() {
    SimulationEngine e;
    northGreen(e);
    e.start();
    for (int i = 0; i < 3; ++i) e.tick();
    CHECK(e.getStatsManager().getProcessedCount() == 0);
    CHECK(e.getCurrentTick() == 3);
}

void test_paused_does_not_dequeue() {
    SimulationEngine e;
    northGreen(e);
    addVehicle(e, Approach::NORTH);
    e.start();
    e.pause();
    e.tick();
    CHECK(e.getQueueManager().getQueueSize(Approach::NORTH) == 1);
    CHECK(e.getStatsManager().getProcessedCount() == 0);
}

void test_reset() {
    SimulationEngine e;
    northGreen(e);
    addVehicle(e, Approach::NORTH);
    addVehicle(e, Approach::EAST);
    e.start();
    e.tick();
    e.reset();
    CHECK(e.getState() == SimState::STOPPED);
    CHECK(e.getCurrentTick() == 0);
    CHECK(e.getQueueManager().getTotalWaitingCount() == 0);
    CHECK(e.getStatsManager().getProcessedCount() == 0);
}

void test_run_simulation() {
    SimulationEngine e;
    e.runSimulation(0);
    CHECK(e.getState() == SimState::STOPPED);
    CHECK(e.getCurrentTick() == 0);
    e.runSimulation(-4);
    CHECK(e.getCurrentTick() == 0);
    e.runSimulation(5);
    CHECK(e.getCurrentTick() == 5);
    CHECK(e.getState() == SimState::RUNNING);
}

void test_last_processed() {
    SimulationEngine e;
    northGreen(e);
    addVehicle(e, Approach::NORTH);
    addVehicle(e, Approach::NORTH);
    CHECK(e.getLastProcessed().empty());
    e.start();
    e.tick();
    CHECK(e.getLastProcessed().size() == 1);
    CHECK(e.getLastProcessed()[0].approach == Approach::NORTH);
    CHECK(e.getLastProcessed()[0].id == 1);          // FIFO: first vehicle out first
    e.tick();
    CHECK(e.getLastProcessed().size() == 1);
    CHECK(e.getLastProcessed()[0].id == 2);
    e.tick();                                         // queue now empty
    CHECK(e.getLastProcessed().empty());
    e.reset();
    CHECK(e.getLastProcessed().empty());
}

int main() {
    test_initial_state();
    test_tick_ignored_when_stopped();
    test_start_and_tick();
    test_pause_resume();
    test_invalid_transitions_are_ignored();
    test_stop_keeps_tick_and_stats();
    test_green_releases_one_vehicle_per_tick();
    test_red_approach_never_dequeues();
    test_empty_queue_on_green_is_safe();
    test_paused_does_not_dequeue();
    test_reset();
    test_run_simulation();
    test_last_processed();

    if (failures == 0) std::cout << "All SimulationEngine tests passed.\n";
    else std::cout << failures << " check(s) failed.\n";
    return failures == 0 ? 0 : 1;
}