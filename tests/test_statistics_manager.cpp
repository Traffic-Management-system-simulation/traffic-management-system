#include "StatisticsManager.hpp"
#include <cmath>
#include <iostream>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << "FAIL: " #cond " (line " << __LINE__ << ")\n";    \
            ++failures;                                                    \
        }                                                                  \
    } while (0)

static bool approx(double a, double b) { return std::fabs(a - b) < 1e-9; }

// Adjust this helper to match your Vehicle struct/constructor
static Vehicle makeVehicle(int entryTick) {
    Vehicle v{};
    v.entryTick = entryTick;
    return v;
}

void test_no_vehicles() {
    StatisticsManager s;
    CHECK(s.getProcessedCount() == 0);
    CHECK(approx(s.calculateAverageWaitTime(), 0.0));
    CHECK(approx(s.calculateThroughput(100), 0.0));
}

void test_average_wait() {
    StatisticsManager s;
    s.recordProcessedVehicle(makeVehicle(0), 4);   // wait 4
    s.recordProcessedVehicle(makeVehicle(2), 8);   // wait 6
    CHECK(s.getProcessedCount() == 2);
    CHECK(s.getTotalWaitTime() == 10);
    CHECK(approx(s.calculateAverageWaitTime(), 5.0));
}

void test_throughput() {
    StatisticsManager s;
    s.recordProcessedVehicle(makeVehicle(0), 1);
    s.recordProcessedVehicle(makeVehicle(0), 2);
    CHECK(approx(s.calculateThroughput(10), 0.2));
}

void test_zero_or_negative_ticks() {
    StatisticsManager s;
    s.recordProcessedVehicle(makeVehicle(0), 1);
    CHECK(approx(s.calculateThroughput(0), 0.0));
    CHECK(approx(s.calculateThroughput(-5), 0.0));
}

void test_negative_wait_clamped() {
    StatisticsManager s;
    s.recordProcessedVehicle(makeVehicle(10), 5);  // bad timestamps
    CHECK(s.getTotalWaitTime() == 0);
    CHECK(s.getProcessedCount() == 1);
}

void test_reset() {
    StatisticsManager s;
    s.recordProcessedVehicle(makeVehicle(0), 3);
    s.reset();
    CHECK(s.getProcessedCount() == 0);
    CHECK(s.getTotalWaitTime() == 0);
    CHECK(approx(s.calculateAverageWaitTime(), 0.0));
}

int main() {
    test_no_vehicles();
    test_average_wait();
    test_throughput();
    test_zero_or_negative_ticks();
    test_negative_wait_clamped();
    test_reset();

    if (failures == 0) std::cout << "All StatisticsManager tests passed.\n";
    else std::cout << failures << " check(s) failed.\n";
    return failures == 0 ? 0 : 1;
}