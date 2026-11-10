// Covers StatisticsManager::printSummary and the average/throughput calculations
#include <cmath>
#include <iostream>
#include <sstream>
#include "StatisticsManager.hpp"

namespace {
int failures = 0;
void check(bool ok, const char* name) {
    std::cout << (ok ? "PASS: " : "FAIL: ") << name << "\n";
    if (!ok) ++failures;
}
}  // namespace

int main() {
    // Empty statistics
    {
        StatisticsManager s;
        std::ostringstream buf;
        std::streambuf* old = std::cout.rdbuf(buf.rdbuf());
        s.printSummary(10);
        s.printSummary(0);
        std::cout.rdbuf(old);
        check(!buf.str().empty(), "printSummary prints output for empty statistics");
        check(s.getProcessedCount() == 0, "no vehicles processed yet");
        check(s.calculateAverageWaitTime() == 0.0, "average wait is 0 with no vehicles");
    }

    // Statistics with data
    {
        StatisticsManager s;
        s.recordProcessedVehicle(Vehicle{1, Approach::NORTH, 2}, 7);   // waited 5
        s.recordProcessedVehicle(Vehicle{2, Approach::EAST, 3}, 8);    // waited 5
        check(s.getProcessedCount() == 2, "two vehicles recorded");
        check(s.getTotalWaitTime() == 10, "total wait time is 10");
        check(std::fabs(s.calculateAverageWaitTime() - 5.0) < 1e-9, "average wait is 5.0");
        check(s.calculateThroughput(10) >= 0.0, "throughput is non-negative");
        check(s.calculateThroughput(0) >= 0.0, "throughput with 0 ticks is safe");

        std::ostringstream buf;
        std::streambuf* old = std::cout.rdbuf(buf.rdbuf());
        s.printSummary(10);
        std::cout.rdbuf(old);
        check(!buf.str().empty(), "printSummary prints output with data");

        s.reset();
        check(s.getProcessedCount() == 0, "reset clears the processed count");
    }

    std::cout << (failures == 0 ? "All statistics summary tests passed.\n"
                                : "Some statistics summary tests FAILED.\n");
    return failures == 0 ? 0 : 1;
}
